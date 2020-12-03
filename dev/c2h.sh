#!/bin/sh
file=$1
dir=$(dirname ${file})/
defname=$(basename ${file} .c | tr '[:lower:]' '[:upper:]')

files="$(sed '/^$/d' ${file} | sed '/#include <.*>/d' | sed 's/#include "\(.*\)"/\1/g')"

printf '#ifndef %s_H\n#define %s_H\n' ${defname} ${defname}
for f in ${files}; do
    base=$(basename ${f} .c | sed "s/\(_rle\|_pb16\|_pb8\|_lz3\)//g")
    grep "${base}[^_]" ${dir}${f} | sed 's/\(.*\)=.*/extern \1;/g' | sed 's/[[:space:]]*;/;/g'
    grep "#define" ${dir}${f}
done
echo "#endif"