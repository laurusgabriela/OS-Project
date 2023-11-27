#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <c>"
    exit 1
fi

c="$1"
count=0
contains=".*$c.*"

while IFS= read -r line; do
    if [[ "$line" =~ ^[A-Z][A-Za-z0-9\ ,.!?\ ]+[\ .!?\ ]+$ && ! "$line" =~ ,\ (si|Si) && "$line" =~ $contains ]]; then
        ((count++))
    fi
done

echo $count

