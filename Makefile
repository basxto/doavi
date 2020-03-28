OBJ=./obj
DEV=./dev
BIN=$(DEV)/gbdk-n/bin

CC=$(BIN)/gbdk-n-compile.sh
LK=$(BIN)/gbdk-n-link.sh
MKROM=makebin -Z -yc
EMU=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=$(DEV)/png2gb/png2gb.py
loadgpl=$(DEV)/loadgpl/loadgpl.py
tmxconvert=$(DEV)/tmx2c.py

ROM=doavi.gb

build: gbdk-n pix/overworld_gbc_data.c pix/overworld_anim_gbc_data.c pix/characters_data.c pix/win_gbc_data.c pix/demo_tmap.c $(ROM)

$(ROM): main.ihx
	$(MKROM) $^ $@

music.gb: mainmusic.ihx
	$(MKROM) $^ $@

run: $(ROM)
	$(EMU) $^

playmusic: music.gb
	$(EMU) $^

main.ihx: main.rel hud.rel music.rel
	$(LK) -o $@ $^

mainmusic.ihx: mainmusic.rel music.rel
	$(LK) -o $@ $^

pix/characters_data.c: pix/angry_toast_gbc.png pix/muffin_gbc.png
	$(pngconvert) --width 2 --height 2 $^ -o $@

pix/win_gbc_data.c: pix/win_gbc.png
	$(pngconvert) $^

%_anim_gbc_data.c: %_anim_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^

%.ihx: %.rel
	$(LK) -o $@ $^

%.rel: %.c
	$(CC) -o $@ $^

%_data.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_map.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_pal.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_tmap.c: %.tmx
	$(tmxconvert) $^

pix/overworld%gb.png: pix/overworld%gbc.png
	$(loadgpl) $^ pix/overworld_gb.gpl $@
	convert $@ $@


%_gb.png: %_gbc.png
	$(loadgpl) $^ pix/gb.gpl $@
	#convert $@ $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f *.gb *.o *.map *.lst *.sym *.rel *.ihx *.lk *.noi *.asm pix/*_gb.png
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete

test: build run

base64:
	base64 $(ROM) | xclip -selection clipboard

wordcount:
	wc -m main.c
