OBJ=./obj
DEV=./dev
BIN=$(DEV)/gbdk-n/bin

CC=$(BIN)/gbdk-n-compile.sh
LK=$(BIN)/gbdk-n-link.sh
MKROM=makebin -Z -yc
EMU=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=$(DEV)/pngconverter.sh
#pngconvert=$(DEV)/cfile_builder.sh
loadgpl=$(DEV)/loadgpl/loadgpl.py
tmxconvert=$(DEV)/tmx2c.py

ROM=doavi.gb

build: gbdk-n pix/overworld_gb_data.c pix/overworld_anim_gb_data.c pix/angry_toast_gb_data.c pix/win_gb_data.c pix/demo_tmap.c $(ROM)

$(ROM): main.ihx
	$(MKROM) $^ $@

run: $(ROM)
	$(EMU) $^

main.ihx: main.rel hud.rel music.rel
	$(LK) -o $@ $^

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

pix/overworld%gb.png: pix/overworld%gbc.png
	$(loadgpl) $^ pix/overworld_gb.gpl $@
	convert $@ $@


%_gb.png: %_gbc.png
	$(loadgpl) $^ pix/gb.gpl $@
	convert $@ $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f *.gb *.o *.map *.lst *.sym *.rel *.ihx *.lk *.noi *.asm pix/*_gb.png
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|tmap\)\.c' -delete

test: build run

base64:
	base64 $(ROM) | xclip -selection clipboard

wordcount:
	wc -m main.c
