#!/bin/bash

IN_FILE="$1"
OUT_FILE="$2"

VAR_NAME=$(basename "$IN_FILE" .cl)

cat << EOF > "$OUT_FILE"
static const char* ${VAR_NAME}_str = R"RAW_OCL(
$(cat "$IN_FILE")
)RAW_OCL";
EOF
