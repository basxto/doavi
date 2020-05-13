#!/bin/bash
set -e
IFS='
'
value=($(grep -o '^[^#]*' "$1"))
name=($(grep -o '^[^#]*' "$1" | sed 's/\\n//g' | sed 's/[^a-z0-9A-Z]//g' | tr '[:upper:]' '[:lower:]'))

# TODO: find most use words and reference them
# grep -o '^[^#]*' strings.txt | tr '[:lower:]' '[:upper:]' | sed 's/\\N/\\n/g' | sed 's/\\X/\\x/g' | sed 's/\\n//g' | tr -c '[:alnum:]' '[\n*]' | grep -E '^.{2,}$' | sort | uniq -c | sort -rk1

address=-1
if [ $# -eq 3 ]; then
    address=$(($3))
fi

echo "// Generated with text2c.sh" > "$2"
#echo "#include \"${2/.c/.h}\"" >> "$2"
echo "#define strlen(x) (sizeof(x) - 1)" >> "${2}"

echo "// Generated with text2c.sh" > "${2/.c/.h}"
echo "#ifndef ${2/.c/_h}" >> "${2/.c/.h}"
echo "#define ${2/.c/_h}" >> "${2/.c/.h}"
echo "#define strlen(x) (sizeof(x) - 1)" >> "${2/.c/.h}"

for i in $(seq 0 $((${#name[@]}-1))); do
    size=$(echo -n "${value[i]}" | sed 's/\\x[0-9]*/./g' | sed 's/\\././g' | sed 's/""//g' | wc -c)
    varname=text_${name[i]:0:8}
    if [ "$address" != "-1" ]; then
        printf "__at (0x%X) " ${address} >> "$2"
        address=$((address + size + 1))
    fi
    if [ "$size" -ge 256 ]; then
        >&2 echo Error: ${varname} has a length of ${size}, which does not fit into UINT8
        exit 1
    fi
    echo const unsigned char ${varname}[$((size+1))] = \""${value[i]}"\"\; >> "$2"
    echo extern const unsigned char ${varname}[$((size+1))]\; >> "${2/.c/.h}"
done

echo "#endif" >> "${2/.c/.h}"