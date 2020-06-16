DEV?=./dev
BIN=$(DEV)/gbdk-n/bin

LK?=$(BIN)/gbdk-n-link.sh -Wl-m
#ROM+MBC1+RAM 4 ROM banks and 4 RAM banks
MKROM?=$(BIN)/gbdk-n-make-rom.sh -yc -yn "DessertOnAVegI" -ya 4 -yt 2 -yo 4
CA=$(BIN)/gbdk-n-assemble.sh
EMU?=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert?=$(DEV)/png2gb/png2gb.py -ci
compress?=$(DEV)/png2gb/compress2bpp.py -ci
pb16?=$(DEV)/pb16.py
loadgpl=$(DEV)/loadgpl/loadgpl.py
gbc2gb?=$(DEV)/gbc2gb.py
rgbgfx?=rgbgfx
xxd?=xxd
tmxconvert=$(DEV)/tmx2c.py
bin2c=$(DEV)/bin2c.sh
convert?=convert

CFLAGS += --peep-file $(abspath $(DEV))/gbz80-ph/peep-rules.txt

COMPRESS?=1

ifeq ($(COMPRESS),1)
CC=$(BIN)/gbdk-n-compile.sh -Wa-l -DCOMPRESS=1 $(CFLAGS)
else
CC=$(BIN)/gbdk-n-compile.sh -Wa-l $(CFLAGS)
endif

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=$(LEVELTMX:.tmx=_tmap.c)
MUSIC=dev/gbdk-music/music/the_journey_begins.c
PIX=$(addprefix pix/,$(addsuffix _data.c,overworld_a_gbc overworld_b_gbc inside_wood_house overworld_anim_gbc overworld_cave characters win_gbc modular_characters))

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

$(DEV)/gbdk-music/%: FORCE
	$(MAKE) -C $(DEV)/gbdk-music $* DEV="../" EMU="$(EMU)"

main.ihx: main.rel hud.rel $(DEV)/gbdk-music/music.rel map.rel logic.rel $(DEV)/png2gb/csrc/decompress.rel level.rel strings.rel music/songs.rel
	$(LK) -o $@ $^

%.ihx: %.rel
	$(LK) -o $@ $^

main.rel: main.c pix/hud_pal.c $(PIX) $(MUSIC) level.c strings.c
	$(CC) -o $@ $<

hud.rel: hud.c pix/dialog_photos_data.c
	$(CC) -o $@ $<

map.rel: map.c $(PIX)
	$(CC) -o $@ $<

$(DEV)/png2gb/%: FORCE
	$(MAKE) DEV=../ -C $(DEV)/png2gb $*

%.rel: %.c
	$(CC) -o $@ $^

%.rel: %.s
	$(CA) -o $@ $^

.PHONY: pix
pix: $(PIX) pix/dialog_photos_data.c

pix/dialog_photos_data.c: pix/dialog_photos.png
	$(pngconvert) --width 4 --height 4 -u yes $^ -o $@ -bin | $(compress) - -o$@

pix/modular_characters_data.c : pix/body_gbc.png  pix/body_ghost_gbc.png $(addprefix pix/head_,$(addsuffix _gbc.png,candy male0 ghost robot0 robot1 female0 male2 rachel))
	$(pngconvert) -flip $^ -o $@

pix/characters_data.c : pix/angry_toast_gbc.png pix/muffin_gbc.png  pix/ghost_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^ -o $@

pix/win_gbc_data.c: pix/win_gbc_rle.xbpp pix/squont8ng.1bpp pix/win_gbc.2bpp
	size=$$(stat --printf="%s" $<);\
	tiles=$$(stat --printf="%s" pix/squont8ng.1bpp);\
	tiles=$$((tiles*2 + $$(stat --printf="%s"  pix/win_gbc.2bpp)));\
	tiles=$$((tiles/16));\
	dev/bin2c.sh $< $@ "imagemagick, rgbgfx and png2gb" $$tiles $$size

pix/win_gbc_rle.xbpp: pix/win_gbc_rle.2bpp pix/squont8ng_rle.1bpp
	cat $^ > $@

# define position in rom
# datrom and palrom have fixed max size
pix/overworld_a_gbc_data.c: pix/overworld_a_gbc.png pix/house_wood_round.png
	$(pngconvert) --width 2 --height 2 --limit 128 $^ -bin | $(compress) - -o$@

pix/overworld_b_gbc_data.c: pix/overworld_b_gbc.png pix/sand_bottle.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

pix/inside_wood_house_data.c: pix/inside_wood_house.png pix/carpet_gbc.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

pix/overworld_cave_data.c: pix/overworld_cave.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

%_anim_gbc_data.c: %_anim_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^

pix/hud_pal.c: pix/win_gbc.png
	$(pngconvert) $^ -o$@

#$(shell printf '0x%X' $$(($(1))))
#$((`stat --printf="%s"  pix/overworld_a.2bpp`/16))
%_data.c: %.2bpp
	$(bin2c) $^ $@ "rgbds and xxd" $$(($$(stat --printf="%s" $$(echo $^ |sed 's/_rle//g'))/16))

%_map.c: %.tilemap
	$(bin2c) $^ $@ "rgbds and xxd"

%_pal.c: %.pal
	$(bin2c) $^ $@ "gbc2gb.py and xxd"

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

%_rle.2bpp: %.2bpp
	dev/png2gb/compress2bpp.py $^ -o $@

%_rle.1bpp: %.1bpp
	dev/png2gb/compress2bpp.py -mono $^ -o $@

%.2bpp %.tilemap: %.png
	$(pngconvert) -cno $< -o $@

%_pb16.2bpp: %.2bpp
	$(pb16) $^ $@

%.pal: %_gbc.png
	$(gbc2gb) $^

%.pal: %.png
	$(gbc2gb) $^

%_gb.png: %_gbc.png
	$(gbc2gb) $^

%_gb.png: %.png
	$(gbc2gb) $^

%.1bpp: %_mono.png
	$(rgbgfx) -d1 $^ -o$@

%_mono.png: %_gbc.png
	$(convert) $^ -monochrome $@

gbdk-n:
	$(MAKE) -C $(DEV)/gbdk-n

clean:
	rm -f pix/*_gb.png level.c strings.c strings.h
	find . -maxdepth 2 -type f -regex '.*.\(gb\|o\|map\|lst\|sym\|rel\|ihx\|lk\|noi\|asm\|adb\|cdb\|bi4\|pal\|2bpp\|1bpp\|tilemap\)' -delete
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete
	find . -maxdepth 2 -type f -regex '.*_\(gb\|mono\)\.png' -delete
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