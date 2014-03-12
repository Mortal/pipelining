#!/bin/bash

if [ "x$1" == "x--help" -o "x$1" == "x" ]; then
	echo "Parameters: [imperative|tpie|stxxl] <input> [output]"
	exit 1
fi

echo "Running on `hostname`"
date
uname -a

kind=""
case "x$1" in
	ximperative|xtpie|xstxxl)
		kind="$1"
		shift
		;;
	*)
		;;
esac


input="$1"
output="$2"
if [ "x$output" == "x" ]; then
	output="`basename "$input" .tif`-transformed.tif"
fi
if ! gdalinfo=`gdalinfo $input`; then
	echo "gdalinfo failed. does input exist?"
	exit 1
fi
width=`sed -nre 's/^Size is ([0-9]+), ([0-9]+)$/\1/p' <<< "$gdalinfo"`
height=`sed -nre 's/^Size is ([0-9]+), ([0-9]+)$/\2/p' <<< "$gdalinfo"`
memory=$((4*1024))

echo "Input is [$input] size [$width x $height]"
echo "Output is [$output]"
echo "Using $memory MB of memory"

function run() {
	"$1" --input "$input" --output "$output" \
		--memory "$memory" --outsize "$width $height" \
		--translate "$((width/2)) $((height/2))" \
		--rotate 32 \
		--translate "-$((width/2)) -$((height/2))"
}

if [ "x$kind" = "ximperative" -o "x$kind" = "x" ]; then
	echo "Running TPIE imperative"
	rm -rf collectl/tpie-imperative/
	rm -f $output

	collectl -sdCM -P -f collectl/tpie-imperative/ & collect=$!
	run build/tpie_imperative/tpie-imperative-transform
	kill $collect
fi

if [ "x$kind" = "xtpie" -o "x$kind" = "x" ]; then
	echo "Running TPIE pipelining"
	rm -rf collectl/tpie/
	rm -f $output

	collectl -sdCM -P -f collectl/tpie/ & collect=$!
	run build/tpie/tpie-transform
	kill $collect
fi

if [ "x$kind" = "xstxxl" -o "x$kind" = "x" ]; then
	echo "Running STXXL"
	rm -rf collectl/stxxl/
	rm -f $output

	collectl -sdCM -P -f collectl/stxxl/ & collect=$!
	run build/stxxl/stxxl-transform
	kill $collect
fi

# Collect results
if [ "x$kind" == "x" ]; then
	tar czf ./`date +"%T"`.tar.gz collectl
fi
