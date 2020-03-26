#!/bin/sh
DIR=`dirname "$(readlink -f "$0")"`
if [ "$#" -ne 1 ];then
	echo "./cfile_builder my.png"
	exit 1
fi
BASE=`basename "$1" .png`
FILEDIR=`dirname "$1"`/

echo "const unsigned char ${BASE}_data[] = {" > "${FILEDIR}${BASE}_data.c"
TILES=`${DIR}/pictotile/pictotile "$1"`
echo "${TILES}" >> "${FILEDIR}${BASE}_data.c"
echo "};" >> "${FILEDIR}${BASE}_data.c"

MAPS=""
counter=0
IFS=,
for i in ${TILES}; do
	MAPS="${MAPS}`printf "0x%X," ${counter}`"
	if [ "$((counter%16))" -eq 15 ];then
		MAPS="${MAPS}\n"
	fi
	let counter++
done

echo "const UINT16 ${BASE}_map[] = {" > "${FILEDIR}${BASE}_map.c"
echo -e "${MAPS}" >> "${FILEDIR}${BASE}_map.c"
echo "};" >> "${FILEDIR}${BASE}_map.c"