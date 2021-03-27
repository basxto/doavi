# for not installed gbdk and sdcc
GBDKBIN=
SDCCBIN=

# globally installed
# use --sdccbin= for a custom sdcc
LCC?=$(GBDKBIN)lcc

CPP=$(LCC) -Wf-E
CPPFLAGS=
CC=$(LCC)
CFLAGS=-Wf--fverbose-asm
AR=$(SDCCBIN)sdar
ARFLAGS=
AS=$(LCC)
ASFLAGS=-c
LD=$(LCC)
LDFLAGS=-Wm-yn"DESSERTONAVEGI" -Wm-yc -Wm-yt0x03 -Wm-ya1 -Wl-j -Wm-yS
VPATH = src

EMU?=sameboy
pngconvert?=./dev/png2gb/png2gb.py -ci
compress?=./dev/png2gb/compress2bpp.py -ci
pb16?=./dev/pb16.py
lz3?=./dev/lzcomp/lzcomp
loadgpl=./dev/loadgpl/loadgpl.py
gbc2gb?=./dev/gbc2gb.py
rgbgfx?=rgbgfx
xxd?=xxd
tmxconvert=./dev/tmx2c.py
bin2c=./dev/bin2c.sh
c2h=./dev/c2h.sh
convert?=convert

NOPEEP?=0
COMPRESS?=1
ROMDEBUG?=0

ifeq ($(NOPEEP),1)
CFLAGS += -Wf--no-peep
endif
ifeq ($(COMPRESS),1)
CFLAGS += -DCOMPRESS=1
endif
ifeq ($(ROMDEBUG), 0)
BANK=
else
BANK= -bo $(ROMDEBUG)
LDFLAGS+= -Wm-yo4
endif

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=$(LEVELTMX:.tmx=_tmap.c)
MUSIC=dev/gbdk-music/music/the_journey_begins.c music/cosmicgem_voadi.c
PIX=$(addprefix pix/,$(addsuffix _data.c,overworld_a_gbc overworld_b_gbc inside_wood_house overworld_anim_gbc overworld_cave characters win_gbc_pb16 modular_characters body_move_a_gbc_pb16 body_move_b_gbc_pb16 body_idle_gbc_pb16 body_stand_gbc_pb16 items_gbc_pb16))

ROM=doavi

########################################################

.PHONY: build run spaceleft clean gbonline

build: $(ROM).gb

$(ROM).gb: main.rel hud.rel ./dev/gbdk-music/music.rel map.rel logic.rel undice.rel unpb16.rel unlz3.rel strings.rel level.rel music/songs.rel pix/pix.rel
	$(LD) $(LDFLAGS) -o $@ $^

%.asm: %.c
	$(CC) $(CFLAGS) -S -o $@ $^

main.asm: main.c pix/pix.h strings.h
	$(CC) $(CFLAGS) -S -o $@ $<

logic.asm: logic.c level.h strings.h
	$(CC) $(CFLAGS) -S -o $@ $<

hud.asm: hud.c pix/pix.h
	$(CC) $(CFLAGS) -S -o $@ $<

map.asm: map.c pix/pix.h music/songs.h
	$(CC) $(CFLAGS) -S -o $@ $<

strings.asm: strings.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

level.asm: level.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

music/songs.asm: music/songs.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

pix/pix.asm: pix/pix.c pix/overworld_a_gbc_pb16_data.c pix/overworld_b_gbc_pb16_data.c pix/overworld_cave_pb16_data.c pix/inside_wood_house_pb16_data.c pix/modular_characters_pb16_data.c pix/dialog_mouths_lz3_data.c pix/dialog_photos_lz3_data.c $(PIX) pix/hud_pal.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $<

# generated
%.rel: %.asm
	$(AS) $(ASFLAGS) -o $@ $^

# handwritten
%.rel: %.s
	$(AS) $(ASFLAGS) -o $@ $^

pix/pix.h: pix/pix.c pix/pix.rel
	$(c2h) $< > $@

music/songs.h: music/songs.c music/songs.rel
	grep "music.h" music/cosmicgem_voadi.c > $@
	$(c2h) $< >> $@

pix/dialog_photos.2bpp: pix/dialog_photos.png
	$(pngconvert) -cno --width 4 --height 4 -u yes $^ -o $@

pix/modular_characters_data.c : pix/body_move_a_gbc.png pix/body_idle_gbc.png pix/body_stand_gbc.png pix/body_gbc.png  pix/body_ghost_gbc.png $(addprefix pix/head_,$(addsuffix _gbc.png,candy male0 ghost robot0 robot1 female0 male2 rachel male6 male1 male3))
	$(pngconvert) -flip $^ -o $@

pix/modular_characters.2bpp : pix/body_move_a_gbc.png pix/body_idle_gbc.png pix/body_stand_gbc.png pix/body_gbc.png  pix/body_ghost_gbc.png $(addprefix pix/head_,$(addsuffix _gbc.png,candy male0 ghost robot0 robot1 female0 male2 rachel male6 male1 male3))
	$(pngconvert) -cno -flip $^ -o $@

pix/characters_data.c : pix/angry_toast_gbc.png pix/muffin_gbc.png  pix/ghost_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^ -o $@

pix/win_gbc.2bpp: pix/win_gbc.png pix/squont8ng_gbc.png
	$(pngconvert) -cno -u yes $^ -o $@

# define position in rom
# datrom and palrom have fixed max size
pix/overworld_a_gbc_data.c: pix/overworld_a_gbc.png pix/overworld_path_gbc.png pix/house_wood_round.png
	$(pngconvert) --width 2 --height 2 --limit 128 $^ -bin | $(compress) - -o$@

pix/overworld_a_gbc.2bpp: pix/overworld_a_gbc.png pix/overworld_path_gbc.png pix/house_wood_round.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

pix/overworld_b_gbc_data.c: pix/overworld_b_gbc.png pix/sand_bottle.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

pix/overworld_b_gbc.2bpp: pix/overworld_b_gbc.png pix/sand_bottle.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

pix/inside_wood_house_data.c: pix/inside_wood_house.png pix/carpet_gbc.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

pix/inside_wood_house.2bpp: pix/inside_wood_house.png pix/carpet_gbc.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

pix/overworld_cave_data.c: pix/overworld_cave.png
	$(pngconvert) --width 2 --height 2 $^ -bin | $(compress) - -o$@

pix/overworld_cave.2bpp: pix/overworld_cave.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

%_anim_gbc_data.c: %_anim_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^

pix/hud_pal.c: pix/win_gbc.png
	$(pngconvert) $^ -o$@

#$(shell printf '0x%X' $$(($(1))))
#$((`stat --printf="%s"  pix/overworld_a.2bpp`/16))
%_data.c: %.2bpp
	$(bin2c) $^ $@ "png2gb.py and xxd" $$(($$(stat --printf="%s" $$(echo $^ |sed 's/_\(rle\|pb16\|pb8\|lz3\)//g'))/16))

%_map.c: %.tilemap
	$(bin2c) $^ $@ "png2gb.py and xxd"

%_pal.c: %.pal
	$(bin2c) $^ $@ "png2gb.py and xxd"

%_tmap.c: %.tmx
	$(tmxconvert) $^

level.c: $(LEVELTMX)
ifeq ($(COMPRESS),1)
	$(tmxconvert) --compress 1 $^
else
	$(tmxconvert) $^
endif
	./dev/worldmap.sh

strings.c strings.h: strings.ini stringmap.txt specialchars.txt
	./dev/ini2c.py $^ -o $@

%.2bpp %.tilemap: %.png
	$(pngconvert) -cno $< -o $@

%_lz3.2bpp: %.2bpp
	$(lz3) $< $@

%_pb16.2bpp: %.2bpp
	$(pb16) $^ $@

%_pb16.1bpp: %.1bpp
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

gbonline:  $(ROM).gb ./dev/GameBoy-Online/
	./dev/patch-gbonline.sh $< ./dev/GameBoy-Online/

./dev/GameBoy-Online/index.html: gbonline
	cp ./dev/GameBoy-Online/index.xhtml $@
	sed '/<?xml version="1.0" encoding="UTF-8"?>/d' -i $@
	sed 's|<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-US">|<html lang="en">|g' -i $@
	sed 's/<style/<meta charset="UTF-8">&/g' -i $@
	sed 's|\(<input .*\)/>|\1>|g' -i $@
	sed 's|\(<\([[:alpha:]]*\) .*\)/>|\1></\2>|g' -i $@
	sed 's|<style type="text/css">@import url(&quot;\(css/GameBoy.css\)&quot;);</style>|<link rel="stylesheet" type="text/css" href="\1">|g' -i $@

# itch.io release
# https://itch.io/docs/creators/html5
%.zip: ./dev/GameBoy-Online/index.html
	cd ./dev/GameBoy-Online/ && zip -r $@ ./js/ ./images/ ./css/ ./index.html
	mv ./dev/GameBoy-Online/$@ .

./dev/gbdk-music/%: FORCE
	$(MAKE) -C ./dev/gbdk-music $* DEV="../" EMU="$(EMU)" CFLAGS='$(CFLAGS)'

./dev/png2gb/%: FORCE
	$(MAKE) DEV=../ -C ./dev/png2gb $*

run: build
	$(EMU) $(ROM).gb

clean:
	$(RM) pix/*_gb.png level.c strings.c strings.h pix/pix.h music/songs.h
	find . -maxdepth 2 -type f -regex '.*.\(gb\|o\|map\|lst\|sym\|rel\|ihx\|lk\|noi\|asm\|adb\|cdb\|bi4\|pal\|2bpp\|1bpp\|xbpp\|tilemap\)' -delete
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete
	find . -maxdepth 2 -type f -regex '.*_\(gb\|mono\)\.png' -delete
	$(MAKE) -C ./dev/gbdk-music clean
	$(MAKE) -C ./dev/png2gb clean

spaceleft: build
	dev/romusage/bin/romusage $(ROM).noi -g

FORCE: