DEV?=./dev
BIN=$(DEV)/gbdk-n/bin

CC=$(BIN)/gbdk-n-compile.sh
LK?=$(BIN)/gbdk-n-link.sh
MKROM?=makebin -Z -yc
EMU?=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=$(DEV)/png2gb/png2gb.py
loadgpl=$(DEV)/loadgpl/loadgpl.py
tmxconvert=$(DEV)/tmx2c.py

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=$(LEVELTMX:.tmx=_tmap.c)
PIX=$(addprefix pix/,$(addsuffix _data.c,overworld_a_gbc overworld_anim_gbc characters win_gbc characters))

ROM=doavi.gb

build: $(ROM)

$(ROM): main.ihx
	$(MKROM) $< $@

run: $(ROM)
	$(EMU) $^

playmusic:
	$(MAKE) -C $(DEV)/gbdk-music playmusic DEV="../" EMU="$(EMU)"

$(DEV)/gbdk-music/music.rel:
	$(MAKE) -C $(DEV)/gbdk-music music.rel DEV="../" EMU="$(EMU)"

main.ihx: main.rel hud.rel $(DEV)/gbdk-music/music.rel
	$(LK) -o $@ $^

%.ihx: %.rel
	$(LK) -o $@ $^

main.rel: main.c $(PIX) level.c
	$(CC) -o $@ $<

hud.rel: hud.c pix/dialog_photos_data.c
	$(CC) -o $@ $<

%.rel: %.c
	$(CC) -o $@ $^

pix/dialog_photos_data.c: pix/dialog_photos.png
	$(pngconvert) --width 4 --height 4 -u yes $^ -o $@

pix/characters_data.c: pix/angry_toast_gbc.png pix/muffin_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^ -o $@

pix/win_gbc_data.c: pix/win_gbc.png
	$(pngconvert) $^

%_anim_gbc_data.c: %_anim_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^

%_data.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_map.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_pal.c: %.png
	$(pngconvert) --width 2 --height 2 $^

%_tmap.c: %.tmx
	$(tmxconvert) $^

level.c: $(LEVEL)
	$(DEV)/worldmap.sh

pix/overworld%gb.png: pix/overworld%gbc.png
	$(loadgpl) $^ pix/overworld_gb.gpl $@
	convert $@ $@

%_gb.png: %_gbc.png
	$(loadgpl) $^ pix/gb.gpl $@
	#convert $@ $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f *.gb *.o *.map *.lst *.sym *.rel *.ihx *.lk *.noi *.asm pix/*_gb.png level.c
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete
	$(MAKE) -C $(DEV)/gbdk-music clean

test: build run

base64:
	base64 $(ROM) | xclip -selection clipboard

gbonline:  $(ROM) $(DEV)/GameBoy-Online/
	$(DEV)/patch-gbonline.sh $< $(DEV)/GameBoy-Online/

$(DEV)/GameBoy-Online/index.html: gbonline
	cp $(DEV)/GameBoy-Online/index.xhtml $@
	sed '/<?xml version="1.0" encoding="UTF-8"?>/d' -i $@
	sed 's|<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-US">|<html lang="en">|g' -i $@
	sed 's/<style/<meta charset="UTF-8">&/g' -i $@
	sed 's|\(<input .*\)/>|\1>|g' -i $@
	sed 's|\(<\([[:alpha:]]*\) .*\)/>|\1></\2>|g' -i $@
	sed 's|<style type="text/css">@import url(&quot;\(css/GameBoy.css\)&quot;);</style>|<link rel="stylesheet" type="text/css" href="\1">|g' -i $@

# itch.io release
# https://itch.io/docs/creators/html5
%.zip: $(DEV)/GameBoy-Online/index.html
	cd $(DEV)/GameBoy-Online/ && zip -r $@ ./js/ ./images/ ./css/ ./index.html
	mv $(DEV)/GameBoy-Online/$@ .

wordcount:
	wc -m main.c
