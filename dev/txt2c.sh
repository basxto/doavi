#!/bin/bash
IFS='
'
value=(`sed 's/^.*$/"&";/g' $1 | tr '[:lower:]' '[:upper:]'`)
name=(`sed 's/[^a-z0-9A-Z]//g' $1 | tr '[:upper:]' '[:lower:]'`)

echo "// Generated with text2c.sh" > $2

for i in `seq 0 $((${#name[@]}-1))`; do
    echo const unsigned char text_${name[i]:0:8}[] = ${value[i]} >> $2
done