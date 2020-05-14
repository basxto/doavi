#!/bin/sh
FROM="$1"
TO="$2"
MADEWITH="$3"
NAME=$(basename $(echo "${TO}" | sed "s/\(\.c\|_rle\)$//g" | sed "s/(\.\|_rle)/_/g"))
echo "// Generated with ${MADEWITH}" >  ${TO}
if [ "${FROM}" != "$(echo ${FROM}|sed 's/_rle//g')" ]; then
    echo "// Compressed with compress2bpp.py" >>  ${TO};
fi
if [ "$#" -ge "5" ]; then
    printf "// %X bytes\n" $5 >> ${TO}
fi
printf "const unsigned char %s[] = {\n" ${NAME} >> ${TO};
cat "${FROM}" | xxd -i >> "${TO}"
echo "};" >> ${TO}
if [ "$#" -ge "4" ]; then
    printf "const unsigned char %s_length = %d;\n" ${NAME} $4 >> ${TO}
fi