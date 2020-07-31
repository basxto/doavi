#!/bin/sh
cd "$(dirname "$(readlink -f "$0")")" || exit 1
cd ../level || exit 1

echo '#include "level.h"' > ../level.c

max_x=0
max_y=0

for l in lvl_*.tmx; do
    x=$(echo "$l" | sed 's/\./_/g' | awk -F_ '{print $2}')
    if [ "$max_x" -lt "$x" ]; then
        max_x=$x
    fi
    y=$(echo "$l" | sed 's/\./_/g' | awk -F_ '{print $3}')
    if [ "$max_y" -lt "$y" ]; then
        max_y=$y
    fi
    echo "#include \"level/lvl_${x}_${y}_tmap.c\"" >> ../level.c
done

echo "const Level level[][$((max_x + 1))] = {" >> ../level.c
for y in $(seq 0 "${max_y}"); do
    printf "\t{\n" >> ../level.c
    for x in $(seq 0 "${max_x}"); do
        file="lvl_${x}_${y}_tmap"
        if [ -f "${file}.c" ]; then
            printf '\t\t{%s_background, %s_collision, %s, %s},\n' "${file}" "${file}" "$(awk '/_tmap_chest/{printf $3}' ../level/${file}.c)" "$(awk '/_tmap_flame/{printf $3}' ../level/${file}.c)" >> ../level.c
        else
            printf "\t\t{0, 0},\n" >> ../level.c
        fi
    done
    printf "\t},\n" >> ../level.c
done

echo "};" >> ../level.c
