#!/bin/bash

SAMPLE_FILE=sample.csv
SAMPLE_COUNT=1000

# Reset sample file
> "$SAMPLE_FILE"

for i in `seq 1 $SAMPLE_COUNT`;
do
	r=$RANDOM

	# datetime (unix timestamp)
	echo -n "$(($r << 16))," >> "$SAMPLE_FILE"
	
	# datetime (human readable)
	echo -n "@$(($r << 16))" | xargs -0 date +"%a %b %d %H:%M:%S %Z %Y" -d | tr "\n" "," >> "$SAMPLE_FILE"
	
	# number (uint8_t)
	echo -n "$(($r >> 8))," >> "$SAMPLE_FILE"
	
	# number (uint16_t)
	echo -n "$(($r))," >> "$SAMPLE_FILE"
	
	# number (uint32_t)
	echo -n "$(($r << 16))," >> "$SAMPLE_FILE"
	
	# number (uint64_t)
	echo -n "$(($r << 48))," >> "$SAMPLE_FILE"
	
	# number (int8_t)
	echo -n "-$(($r >> 9))," >> "$SAMPLE_FILE"
	
	# number (int16_t)
	echo -n "-$(($r >> 1))," >> "$SAMPLE_FILE"
	
	# number (uint32_t)
	echo -n "-$(($r << 15))," >> "$SAMPLE_FILE"
	
	# number (uint64_t)
	echo -n "-$(($r << 47))," >> "$SAMPLE_FILE"
	
	# number (float)
	echo $r*1.123 | bc  >> "$SAMPLE_FILE"
	
done
