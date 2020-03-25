OBJ=./obj
DEV=./dev
BIN=$(DEV)/gbdk-n/bin

CC=$(BIN)/gbdk-n-compile.sh
LK=$(BIN)/gbdk-n-link.sh
MKROM=makebin -Z -yc
EMU=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=$(DEV)/pngconverter.sh
loadgpl=$(DEV)/loadgpl/loadgpl.py
tmxconvert=$(DEV)/tmx2c.py

build: gbdk-n pix/overworld_gb_data.c pix/demo_tmap.c main.gb

%.gb: %.ihx
	$(MKROM) $^ $@

run: main.gb
	$(EMU) ./main.gb

%.ihx: %.rel
	$(LK) -o $@ $^

%.rel: %.c
	$(CC) -o $@ $^

%_data.c: %.png
	$(pngconvert) $^

%_map.c: %.png
	$(pngconvert) $^

%_tmap.c: %.tmx
	$(tmxconvert) $^

pix/overworld_gb.png: pix/overworld_gbc.png
	$(loadgpl) $^ pix/gb.gpl $@
	convert $@ $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f *.gb *.o *.map *.lst *.sym *.rel *.ihx pix/overworld_gb.png
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|tmap\)\.c' -delete

test: build run

base64:
	base64 main.gb | xclip -selection clipboard

wordcount:
	wc -m main.c
