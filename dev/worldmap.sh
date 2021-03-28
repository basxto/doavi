#!/bin/sh
cd "$(dirname "$(readlink -f "$0")")" || exit 1
cd ..
output=$1
shift
indir=$(dirname $1)

echo '#include "../src/level.h"' > ${output}

max_x=0
max_y=0

for l in $@; do
    x=$(echo "$l" | sed 's/\./_/g' | awk -F_ '{print $2}')
    if [ "$max_x" -lt "$x" ]; then
        max_x=$x
    fi
    y=$(echo "$l" | sed 's/\./_/g' | awk -F_ '{print $3}')
    if [ "$max_y" -lt "$y" ]; then
        max_y=$y
    fi
    echo "#include \"$(basename "$l")\"" >> ${output}
done

echo "const Level level[][$((max_x + 1))] = {" >> ${output}
for y in $(seq 0 "${max_y}"); do
    printf "\t{\n" >> ${output}
    for x in $(seq 0 "${max_x}"); do
        file="lvl_${x}_${y}_tmap"
        if [ -f "${indir}/${file}.c" ]; then
            printf '\t\t{%s_background, %s_collision, %s, %s},\n' "${file}" "${file}" "$(awk '/_tmap_chest/{printf $3}' "${indir}/${file}.c")" "$(awk '/_tmap_flame/{printf $3}' "${indir}/${file}.c")" >> ${output}
        else
            printf "\t\t{0, 0, 0, 0},\n" >> ${output}
        fi
    done
    printf "\t},\n" >> ${output}
done

echo "};" >> ${output}
