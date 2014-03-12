import sys
import gzip
import tarfile
import heapq
import collections as co
import re
import itertools as itt

def handle_csv_timestamped(fp, handler):
    """Read all lines in a CSV, computing timestamp information
    and sending each line to `handler`."""
    keys = ()
    first_time = None
    prev_time = None
    for line in fp:
        if line.startswith('# ') or line.startswith('##'):
            continue
        if line.startswith('#'):
            keys = line[1:].split()
            continue
        values = line.split()
        data = dict(zip(keys, values))
        timestamp = data['Time']
        hours, minutes, seconds = map(int, timestamp.split(':'))
        abs_time = (hours * 60 + minutes) * 60 + seconds
        if first_time is None:
            first_time = abs_time
            rel_time = 0
        else:
            rel_time = abs_time - first_time

            # Assume duration since last sample is >= 0 seconds and < 1 day
            rel_time = prev_time + (rel_time - prev_time) % (24 * 60 * 60)

        prev_time = rel_time

        yield handler(rel_time, data)

class TimedData(object):
    def __init__(self, time, data):
        self.time = time
        self.data = data

    def __lt__(self, other):
        return self.time < other.time

    def __str__(self):
        return str((self.time, self.data))

### Handlers for different kinds of CSV files depending on filename

handlers = []
def file_handler(trigger):
    def decorator(fun):
        handlers.append((trigger, fun))
        return fun
    return decorator

@file_handler('.cpu')
def handle_cpu_data(fp):
    def handle_line(rel_time, data):
        d = {}
        for k, header in (('cpu', 'Totl%'), ('soft', 'Soft%'), ('wait', 'Wait%'), ('sys', 'Sys%')):
            d[k] = sum(int(v) for k, v in data.items() if k.endswith(header)) / 100
        return TimedData(rel_time, d)

    return handle_csv_timestamped(fp, handle_line)

@file_handler('.numa')
def handle_memory_data(fp):
    def handle_line(rel_time, data):
        memory = sum(int(v) for k, v in data.items() if k.endswith('Inactive'))
        return TimedData(rel_time, {'memory': memory})

    return handle_csv_timestamped(fp, handle_line)

@file_handler('.tab')
def handle_disk_data(fp):
    def handle_line(rel_time, data):
        disk = sum(int(v) for k, v in data.items() if k.endswith('KbTot'))
        return TimedData(rel_time, {'disk': disk})

    return handle_csv_timestamped(fp, handle_line)

def get_handler(filename):
    """Get the handler and reading mode for a given filename,
    or return None."""
    if filename.endswith('.gz'):
        hd, mode = get_handler(filename[:-3])
        if hd:
            def gzip_hd(fp_outer):
                with gzip.open(fp_outer, mode) as fp_inner:
                    yield from hd(fp_inner)
            return gzip_hd, 'rb'
    else:
        for trigger, fun in handlers:
            if trigger in filename:
                return fun, 'rt'
    return None, None

def handle_tar(filename):
    """Handles a tar by combining the handling of files.

    Each file foo/bar/mode/hostname-yyyymmdd.kind.gz is treated as the
    specific `kind` of data resulting from running in the specific `mode`.

    Different kinds of data for each `mode` is grouped by unioning the data
    dictionaries common to each time point.
    """
    with tarfile.open(filename) as tf:

        # Dictionary containing tarinfos for each mode.
        modes = co.defaultdict(list)

        for tarinfo in tf.getmembers():
            # Get handler for this file.
            handler, mode = get_handler(tarinfo.name)
            if not handler:
                continue
            # Assume filename matches the given pattern.
            o = re.match(r'(.*/)?(?P<mode>[^/]*)/(?P<hostname>[^/.]*)-[0-9]{8}\.[a-z]*\.gz', tarinfo.name)
            mode = o.group('mode') if o else ''
            # Store handler with the given mode.
            modes[mode].append((tarinfo, handler))

        def handle_mode(mode, data):
            # unzip `data`
            tarinfos, handlers = zip(*data)

            try:
                # Extract all files from tar simulatenously
                fps = map(tf.extractfile, tarinfos)
                # Handlers for all the extracted files
                datas = map(lambda handler, fp: handler(fp), handlers, fps)
                # Assume data points are increasing in time and merge them by time
                datas_merged = heapq.merge(*datas)
                # Group by time
                datas_grouped = itt.groupby(datas_merged, lambda td: td.time)

                for time, datas in datas_grouped:
                    datas_list = list(datas)
                    data_union = dict(sum((tuple(td.data.items()) for td in datas_list), ()))
                    yield TimedData(datas_list[0].time, data_union)

            finally:
                for fp in fps:
                    fp.close()

        for mode, data in modes.items():
            yield (mode, handle_mode(mode, data))

def print_lines(lines):
    ns = co.defaultdict(int)
    sums = co.defaultdict(int)
    maxs = dict()
    for line in lines:
        #print("{time} {data}".format(time=line.time,
        #    data=" ".join("%s=%s" % (k, v)
        #        for k, v in sorted(line.data.items(), key=lambda kv: kv[0]))))
        for k, v in line.data.items():
            ns[k] += 1
            sums[k] += v
            maxs[k] = max(maxs[k], v) if k in maxs else v
    for k, v in sums.items():
        print("{key}: n={n}, sum={sum}, avg={avg}, max={max}".format(
            key=k, n=ns[k], sum=sums[k], avg=sums[k]/ns[k], max=maxs[k]))

def main(filename):
    if '.tar' in filename:
        for name, mode in handle_tar(filename):
            print("# Inside tar: {name}".format(name=name))
            print_lines(mode)
            print("")
    else:
        handler, mode = get_handler(filename)
        if handler:
            with open(filename, mode) as fp:
                print_lines(handler(fp))

if __name__ == '__main__':
    main(*sys.argv[1:])
