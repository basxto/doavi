#!/bin/sh
cd `dirname "$(readlink -f "$0")"`
cd ../level

echo '#include "level.h"' > ../level.c

max_x=0
max_y=0

for l in lvl_*.tmx; do
    x=`echo $l | sed 's/\./_/g' | awk -F_ '{print $2}'`
    if [ "$max_x" -lt "$x" ]; then
        max_x=$x
    fi
    y=`echo $l | sed 's/\./_/g' | awk -F_ '{print $3}'`
    if [ "$max_y" -lt "$y" ]; then
        max_y=$y
    fi
    echo "#include \"level/lvl_${x}_${y}_tmap.c\"" >> ../level.c
done

echo "Level level[][$((max_x + 1))] = {" >> ../level.c
for y in `seq 0 ${max_y}`; do
    echo -e "\t{" >> ../level.c
    for x in `seq 0 ${max_x}`; do
        file=lvl_${x}_${y}_tmap
        if [ -f "${file}.c" ]; then
            echo -e "\t\t{${file}_background, ${file}_collision}," >> ../level.c
        else
            echo -e "\t\t{0, 0}," >> ../level.c
        fi
    done
    echo -e "\t}," >> ../level.c
done

echo "};" >> ../level.c
