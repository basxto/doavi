#!/bin/bash
set -e
IFS='
'

outfile="${2/.h/.c}"
infile="${1}"

value=($(grep -o '^[^#]*' "${infile}"))
name=($(grep -o '^[^#]*' "${infile}" | sed 's/\\n//g' | sed 's/[^a-z0-9A-Z]//g' | tr '[:upper:]' '[:lower:]'))

# TODO: find most use words and reference them
# grep -o '^[^#]*' strings.txt | tr '[:lower:]' '[:upper:]' | sed 's/\\N/\\n/g' | sed 's/\\X/\\x/g' | sed 's/\\n//g' | tr -c '[:alnum:]' '[\n*]' | grep -E '^.{2,}$' | sort | uniq -c | sort -rk1

address=-1
if [ $# -eq 3 ]; then
    address=$(($3))
fi

echo "// Generated with text2c.sh" > "${outfile}"
#echo "#include \"${outfile/.c/.h}\"" >> "${outfile}"
echo "#define strlen(x) (sizeof(x) - 1)" >> "${outfile}"

echo "// Generated with text2c.sh" > "${outfile/.c/.h}"
echo "#ifndef ${outfile/.c/_h}" >> "${outfile/.c/.h}"
echo "#define ${outfile/.c/_h}" >> "${outfile/.c/.h}"
echo "#define strlen(x) (sizeof(x) - 1)" >> "${outfile/.c/.h}"

for i in $(seq 0 $((${#name[@]}-1))); do
    size=$(echo -n "${value[i]}" | sed 's/\\x[0-9]*/./g' | sed 's/\\././g' | sed 's/""//g' | wc -c)
    varname=text_${name[i]:0:8}
    if [ "$address" != "-1" ]; then
        printf "__at (0x%X) " ${address} >> "${outfile}"
        address=$((address + size + 1))
    fi
    if [ "$size" -ge 256 ]; then
        >&2 echo Error: ${varname} has a length of ${size}, which does not fit into UINT8
        exit 1
    fi
    echo const unsigned char ${varname}[$((size+1))] = \""${value[i]}"\"\; >> "${outfile}"
    echo extern const unsigned char ${varname}[$((size+1))]\; >> "${outfile/.c/.h}"
done

echo "#endif" >> "${outfile/.c/.h}"