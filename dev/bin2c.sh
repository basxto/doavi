#!/bin/sh
FROM="$1"
TO="$2"
MADEWITH="$3"
NAME=$(basename $(echo "${TO}" | sed "s/\(\.c\|_rle\|_pb16\|_pb8\|_lz3\)$//g" | sed "s/\./_/g" | sed "s/_\(rle\|pb16\|pb8\|lz3\)//g"))
echo "// Generated with ${MADEWITH}" >  ${TO}
if [ "${FROM}" != "$(echo ${FROM}|sed 's/_rle//g')" ]; then
    echo "// Compressed with compress2bpp.py" >>  ${TO};
fi
if [ "${FROM}" != "$(echo ${FROM}|sed 's/_pb16//g')" ]; then
    echo "// Compressed with pb16.py" >>  ${TO};
fi
if [ "${FROM}" != "$(echo ${FROM}|sed 's/_pb8//g')" ]; then
    echo "// Compressed with pb8.py" >>  ${TO};
fi
if [ "${FROM}" != "$(echo ${FROM}|sed 's/_lz2//g')" ]; then
    echo "// Compressed with lzcomp" >>  ${TO};
fi
if [ "$#" -ge "5" ]; then
    printf "// %X bytes\n" $5 >> ${TO}
fi
printf "const unsigned char %s[] = {\n" ${NAME} >> ${TO};
cat "${FROM}" | xxd -i >> "${TO}"
echo "};" >> ${TO}
if [ "$#" -ge "4" ]; then
    printf "#define %s_length (%d)\n" ${NAME} $4 >> ${TO}
fi