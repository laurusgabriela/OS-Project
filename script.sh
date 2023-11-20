#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: bash script <c>"
    exit 1
fi

char="$1"
count=0
while IFS= read -r line || [ -n "$line " ];
do
    if [[$line =~ ^[A-Z][a-zA-Z0-9 ,.!]*[A-Z]$ && ! $line =! ,\ and \ ]];
    then
	((count++))
    fi
done

echo "$count"
