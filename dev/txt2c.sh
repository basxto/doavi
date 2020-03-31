#!/bin/bash
IFS='
'
value=(`grep -o '^[^#]*' $1 | tr '[:lower:]' '[:upper:]'`)
name=(`grep -o '^[^#]*' $1 | sed 's/[^a-z0-9A-Z]//g' | tr '[:upper:]' '[:lower:]'`)

echo "// Generated with text2c.sh" > $2
echo "#define strlen(x) (sizeof(x) - 1)" >> $2

for i in `seq 0 $((${#name[@]}-1))`; do
    echo const unsigned char text_${name[i]:0:8}[] = \"${value[i]}\"\; >> $2
done