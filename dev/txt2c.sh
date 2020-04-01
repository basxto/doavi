#!/bin/bash
set -e
IFS='
'
value=($(grep -o '^[^#]*' "$1" | sed 's/\\N/\\n/g'))
name=($(grep -o '^[^#]*' "$1" | sed 's/\\n//g' | sed 's/[^a-z0-9A-Z]//g' | tr '[:upper:]' '[:lower:]'))

address=-1
if [ $# -eq 3 ]; then
    address=$(($3))
fi

echo "// Generated with text2c.sh" > "$2"
echo "#define strlen(x) (sizeof(x) - 1)" >> "$2"

for i in $(seq 0 $((${#name[@]}-1))); do
    size=$(echo -n "${value[i]}" | sed 's/\\n/./g' | wc -c)
    varname=text_${name[i]:0:8}
    if [ "$address" != "-1" ]; then
        printf "__at (0x%X) " ${address} >> "$2"
        address=$((address + size + 1))
    fi
    if [ "$size" -ge 256 ]; then
        >&2 echo Error: ${varname} has a length of ${size}, which does not fit into UINT8
        exit 1
    fi
    echo const unsigned char ${varname}[] = \""${value[i]}"\"\; >> "$2"
done