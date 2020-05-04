#!/bin/sh
saved=0
before=0
after=0
count(){
    before=0
    after=0
    for i in $(echo "${output}" | grep 'Before' | cut -d' ' -f3); do
        before=$((before + i))
    done

    for i in $(echo "${output}" | grep 'After' | cut -d' ' -f3); do
        after=$((after + i))
    done

    printf " Before: 0x%X bytes or 0x%X KiB\n" ${before} $((before/1024))
    printf " After: 0x%X bytes or 0x%X KiB\n" ${after} $((after/1024))
    echo " Shrunk to $((after*100/before))% of it's size!"
    saved=$((before-after))
    printf " %d bytes or %d KiB saved!\n" $saved $((saved/1024))
    echo ""
}

output=$(make -B pix | sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g" | grep '\(Before\|After\) compression:.*bytes')
echo "Only compressed images:"
count

output=$(make -B pix pngconvert="dev/png2gb/png2gb.py -s yes" | sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g" | grep '\(Before\|After\) compression:.*bytes')
echo "All images:"
count

cd dev/png2gb/
compressor=$(DEV=../ ./measure_size.sh | tail -1)

echo "$((saved-compressor)) bytes gained if you consider the compressor's size"
echo "Shrunk to $(((after+compressor)*100/before))%"