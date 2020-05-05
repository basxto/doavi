DEV?=./dev
BIN=$(DEV)/gbdk-n/bin

LK?=$(BIN)/gbdk-n-link.sh -Wl-m
#ROM+MBC1+RAM 4 ROM banks and 4 RAM banks
MKROM?=$(BIN)/gbdk-n-make-rom.sh -yc -yn "DessertOnAVegI" -ya 4 -yt 2 -yo 4
EMU?=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert?=$(DEV)/png2gb/png2gb.py
loadgpl=$(DEV)/loadgpl/loadgpl.py
tmxconvert=$(DEV)/tmx2c.py

COMPRESS?=1

ifeq ($(COMPRESS),1)
CC=$(BIN)/gbdk-n-compile.sh -Wa-l -DCOMPRESS=1 $(CFLAGS)
else
CC=$(BIN)/gbdk-n-compile.sh -Wa-l $(CFLAGS)
endif

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=$(LEVELTMX:.tmx=_tmap.c)
MUSIC=dev/gbdk-music/music/the_journey_begins.c
PIX=$(addprefix pix/,$(addsuffix _data.c,overworld_a_gbc overworld_b_gbc inside_wood_house overworld_anim_gbc overworld_cave characters win_gbc characters))

define calc_hex
$(shell printf '0x%X' $$(($(1))))
endef

ROM=doavi.gb

build: $(ROM)

$(ROM): main.ihx
	$(MKROM) $< $@

run: $(ROM)
	$(EMU) $^

playmusic:
	$(MAKE) -C $(DEV)/gbdk-music playmusic DEV="../" EMU="$(EMU)"

$(DEV)/gbdk-music/music.rel: $(DEV)/gbdk-music/music.c
	$(MAKE) -C $(DEV)/gbdk-music music.rel DEV="../" EMU="$(EMU)"

main.ihx: main.rel hud.rel $(DEV)/gbdk-music/music.rel map.rel logic.rel $(DEV)/png2gb/csrc/decompress.rel
	$(LK) -o $@ $^

%.ihx: %.rel
	$(LK) -o $@ $^

main.rel: main.c $(PIX) $(MUSIC) level.c strings.c
	$(CC) -o $@ $<

hud.rel: hud.c pix/dialog_photos_data.c
	$(CC) -o $@ $<

map.rel: map.c $(PIX)
	$(CC) -o $@ $<

$(DEV)/png2gb/%: FORCE
	$(MAKE) DEV=../ -C $(DEV)/png2gb $*

%.rel: %.c
	$(CC) -o $@ $^

.PHONY: pix
pix: $(PIX) pix/dialog_photos_data.c

pix/dialog_photos_data.c: pix/dialog_photos.png
	$(pngconvert) --compress-rle=1 --width 4 --height 4 -u yes $^ -o $@

pix/characters_data.c: pix/angry_toast_gbc.png pix/muffin_gbc.png  pix/ghost_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^ -o $@

pix/win_gbc_data.c: pix/win_gbc.png
	$(pngconvert) --compress-rle=1 $^

# define position in rom
# datrom and palrom have fixed max size
pix/overworld_a_gbc_data.c: pix/overworld_a_gbc.png pix/house_wood_round.png
	$(pngconvert) --compress-rle=1 --width 2 --height 2 --limit 128 $^

pix/overworld_b_gbc_data.c: pix/overworld_b_gbc.png pix/sand_bottle.png
	$(pngconvert) --compress-rle=1 --width 2 --height 2 $^

pix/inside_wood_house_data.c: pix/inside_wood_house.png
	$(pngconvert) --compress-rle=1 --width 2 --height 2 $^

pix/overworld_cave_data.c: pix/overworld_cave.png
	$(pngconvert) --compress-rle=1 --width 2 --height 2 $^

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

level.c: $(LEVELTMX)
ifeq ($(COMPRESS),1)
	$(tmxconvert) --compress 1 $^
else
	$(tmxconvert) $^
endif
	$(DEV)/worldmap.sh

strings.c: strings.txt
	$(DEV)/txt2c.sh $^ $@

pix/overworld%gb.png: pix/overworld%gbc.png
	$(loadgpl) $^ pix/overworld_gb.gpl $@
	convert $@ $@

%_gb.png: %_gbc.png
	$(loadgpl) $^ pix/gb.gpl $@
	#convert $@ $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f pix/*_gb.png level.c strings.c strings.h
	find . -maxdepth 2 -type f -regex '.*.\(gb\|o\|map\|lst\|sym\|rel\|ihx\|lk\|noi\|asm\|adb\|cdb\|bi4\)' -delete
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete
	$(MAKE) -C $(DEV)/gbdk-music clean
	$(MAKE) -C $(DEV)/png2gb clean

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

rompng:
	$(DEV)/gb2png/gb2png.py doavi.gb --byteoffset 15

### tests for building banks
pix/overworld_a_test_gbc_data.c: pix/overworld_a_gbc.png
	$(pngconvert) --width 2 --height 2 $^ -o pix/overworld_a_test_gbc

pix/overworld_b_test_gbc_data.c: pix/overworld_b_gbc.png
	$(pngconvert) --width 2 --height 2 $^ -o pix/overworld_b_test_gbc

banking.gb: banking.ihx
	$(MKROM) $< $@

banking.bank: banking.gb
	dd skip=`printf "%d" 0x3FFF` count=`printf "%d" 0x4000` if=$^ of=$@ bs=1

banking.ihx: nomain.rel pix/overworld_a_test_gbc_data.rel pix/overworld_b_test_gbc_data.rel
	$(LK) -o $@ $^

bigrom.gb: $(ROM) banking.bank
	cat $^ > $@

runbigrom: bigrom.gb
	$(EMU) $^

.PHONY: spaceleft
spaceleft: $(ROM)
	@hexdump -v -e '/1 "%02X\n"' $(ROM) | awk '/FF/ {n += 1} !/FF/ {n = 0} END {print n}'

FORCE: