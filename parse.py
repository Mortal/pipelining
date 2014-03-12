import sys
import gzip
import tarfile

def handle_csv_timestamped(fp, handler):
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

        handler(rel_time, data)
        prev_time = rel_time

handlers = []
def file_handler(trigger):
    def decorator(fun):
        handlers.append((trigger, fun))
        return fun
    return decorator

@file_handler('.cpu')
def handle_cpu_data(fp):
    def handle_line(rel_time, data):
        cpu = sum(int(v) for k, v in data.items() if k.endswith('Totl%'))
        print("{time} {cpu}".format(time=rel_time, cpu=cpu))

    handle_csv_timestamped(fp, handle_line)

@file_handler('.numa')
def handle_memory_data(fp):
    def handle_line(rel_time, data):
        memory = sum(int(v) for k, v in data.items() if k.endswith('Inactive'))
        print("{time} {memory}".format(time=rel_time, memory=memory))

    handle_csv_timestamped(fp, handle_line)

@file_handler('.tab')
def handle_disk_data(fp):
    def handle_line(rel_time, data):
        disk = sum(int(v) for k, v in data.items() if k.endswith('KbTot'))
        print("{time} {disk}".format(time=rel_time, disk=disk))

    handle_csv_timestamped(fp, handle_line)

def get_handler(filename):
    if filename.endswith('.gz'):
        hd, mode = get_handler(filename[:-3])
        if hd:
            return lambda fp: hd(gzip.open(fp, mode)), 'rb'
    else:
        for trigger, fun in handlers:
            if trigger in filename:
                return fun, 'rt'
    return None, None

def handle_tar(filename):
    with tarfile.open(filename) as tf:
        for tarinfo in tf.getmembers():
            handler, mode = get_handler(tarinfo.name)
            if handler:
                print("# Inside tar: {filename}".format(filename=tarinfo.name))
                with tf.extractfile(tarinfo) as fp:
                    handler(fp)
                print("")

def main(filename):
    if '.tar' in filename:
        handle_tar(filename)
    else:
        handler, mode = get_handler(filename)
        if handler:
            with open(filename, mode) as fp:
                handler(fp)

if __name__ == '__main__':
    main(*sys.argv[1:])
