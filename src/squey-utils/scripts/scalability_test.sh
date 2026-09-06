#!/bin/bash

test $# -lt 1 && echo "usage: `basename $0` program arg1 arg2 arg3..." && exit 1
 
command -v taskset >/dev/null 2>&1 || { echo >&2 "this script needs 'taskset' to be installed"; exit 2; } 
command -v nproc >/dev/null 2>&1 || { echo >&2 "this script needs 'nproc' to be installed"; exit 3; }

export TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD=99999999999

for i in `seq 0 $(($(nproc)-1))`; do taskset -c `seq --separator="," 0 $i` "$@" && echo "" ; done
