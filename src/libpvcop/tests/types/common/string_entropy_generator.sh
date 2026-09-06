#!/bin/bash

test $# -ne 5 && echo "usage: `basename $0` <output_file> <line_count> <distinct_count> <min_line_size> <max_line_size>" && exit 1

output_file="$1"
line_count="$2"
distinct_count="$3"
min_line_size="$4"
max_line_size="$5"
tmp_file="$output_file".tmp

if [ $distinct_count -gt $line_count ]
then
	echo "error : line count should be greater than distinct count"
	exit
fi

# clear output file
> "$output_file"

# generate distinct values
if [ $min_line_size -gt $max_line_size ]
then
	tmp=$max_line_size
	max_line_size=$min_line_size
	min_line_size=$tmp
fi
for i in $(seq 1 $distinct_count)
do
	size=$(shuf -i $min_line_size-$max_line_size -n 1)
	echo $(pwgen $size -1) >> "$tmp_file"
done

# duplicate distinct values
dupl=$((line_count / distinct_count))
for i in $(seq 1 $dupl)
do
	cat "$tmp_file" >> "$output_file"
done

# finish file
remaining_lines=$((line_count - dupl * distinct_count))
head -n $remaining_lines "$tmp_file" >> "$output_file"

# clean temp file
rm -rf "$tmp_file"