DEV?=./dev
BIN=$(DEV)/gbdk-n/bin
SDCCBIN=
GBDKDIR=/opt/gbdk-2020/
GBDKLIB=$(GBDKDIR)/lib/small/asxxxx/

# globally installed
# use --sdccbin= for a custom sdcc
LCC?=lcc -Wa-l
# 0x0143 is gameboy mode
MKROM?=$(SDCCBIN)makebin -Z -yc
CA=$(SDCCBIN)sdasgb -plosgff
LD=$(SDCCBIN)sdldgb
EMU?=sameboy
pngconvert?=$(DEV)/png2gb/png2gb.py -ci
compress?=$(DEV)/png2gb/compress2bpp.py -ci
pb16?=$(DEV)/pb16.py
lz3?=$(DEV)/lzcomp/lzcomp
loadgpl=$(DEV)/loadgpl/loadgpl.py
gbc2gb?=$(DEV)/gbc2gb.py
rgbgfx?=rgbgfx
xxd?=xxd
tmxconvert=$(DEV)/tmx2c.py
bin2c=$(DEV)/bin2c.sh
c2h=$(DEV)/c2h.sh
convert?=convert

NOPEEP?=0

ifeq ($(NOPEEP),1)
CFLAGS += --no-peep
else
CFLAGS += --peep-file$(abspath $(DEV))/gbz80-ph/combined-peeph.def
endif

COMPRESS?=1
ROMDEBUG?=0

ifeq ($(COMPRESS),1)
CC=$(SDCCBIN)sdcc -mgbz80 --fsigned-char --no-std-crt0 -I "$(GBDKDIR)/include" -I "$(GBDKDIR)/include/asm" -c -DCOMPRESS=1 $(CFLAGS)
else
CC=$(SDCCBIN)sdcc -mgbz80 --fsigned-char --no-std-crt0 -I "$(GBDKDIR)/include" -I "$(GBDKDIR)/include/asm" -c $(CFLAGS)
endif

ifeq ($(ROMDEBUG), 0)
BANK=
MKROM+= -yt 0x03 -ya 1
else
BANK= -bo $(ROMDEBUG)
MKROM+= -yt 0x03 -ya 1 -yo 4
endif

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=$(LEVELTMX:.tmx=_tmap.c)
MUSIC=dev/gbdk-music/music/the_journey_begins.c music/cosmicgem_voadi.c
PIX=$(addprefix pix/,$(addsuffix _data.c,overworld_a_gbc overworld_b_gbc inside_wood_house overworld_anim_gbc overworld_cave characters win_gbc_pb16 modular_characters body_move_a_gbc_pb16 body_move_b_gbc_pb16 body_idle_gbc_pb16 body_stand_gbc_pb16 items_gbc_pb16))

define calc_hex
$(shell printf '0x%X' $$(($(1))))
endef

ROM=doavi.gb

.PHONY: build
build: $(DEV)/gbz80-ph/combined-peeph.def $(ROM)

$(DEV)/gbz80-ph/%.def: FORCE
	$(MAKE) -C $(DEV)/gbz80-ph/ $*.def

$(ROM): $(ROM).ihx
	$(DEV)/noi2sym.sh $(ROM).noi $$(basename $(ROM) .gb).sym
	$(MKROM) -yn "DESSERTONAVEGI" $^ $@

$(ROM).ihx: main.rel hud.rel $(DEV)/gbdk-music/music.rel map.rel logic.rel undice.rel unpb16.rel unlz3.rel strings.rel level.rel music/songs.rel pix/pix.rel
	$(LD) -nmjwxi -k "$(GBDKLIB)/gbz80/" -l gbz80.lib -k "$(GBDKLIB)/gb/" -l gb.lib -g .OAM=0xC000 -g .STACK=0xE000 -g .refresh_OAM=0xFF80 -g .init=0x000 -b _DATA=0xc0a0 -b _CODE=0x0200 $@ "${GBDKDIR}/lib/small/asxxxx/gb/crt0.o" $^

.PHONY: run
run: build
	$(EMU) $(ROM)

$(DEV)/gbdk-music/%: FORCE
	$(MAKE) -C $(DEV)/gbdk-music $* DEV="../" EMU="$(EMU)" CFLAGS='$(CFLAGS)'

main.asm: main.c pix/pix.h strings.h
	$(CC) --fverbose-asm -S -o $@ $<

logic.asm: logic.c level.h strings.h
	$(CC) --fverbose-asm -S -o $@ $<

hud.asm: hud.c pix/pix.h
	$(CC) --fverbose-asm -S -o $@ $<

map.asm: map.c pix/pix.h music/songs.h
	$(CC) --fverbose-asm -S -o $@ $<

$(DEV)/png2gb/%: FORCE
	$(MAKE) DEV=../ -C $(DEV)/png2gb $*

strings.rel: strings.c
	$(CC) $(BANK) -o $@ $^

level.rel: level.c
	$(CC) $(BANK) -o $@ $^

music/songs.rel: music/songs.c
	$(CC) $(BANK) -o $@ $^

%.asm: %.c
	$(CC) --fverbose-asm -S -o $@ $^

# generated
%.rel: %.asm
	$(CA) -o $@ $^

# handwritten
%.rel: %.s
	$(CA) -o $@ $^

pix/pix.rel:pix/pix.c pix/overworld_a_gbc_pb16_data.c pix/overworld_b_gbc_pb16_data.c pix/overworld_cave_pb16_data.c pix/inside_wood_house_pb16_data.c pix/modular_characters_pb16_data.c pix/dialog_mouths_lz3_data.c pix/dialog_photos_lz3_data.c $(PIX) pix/hud_pal.c
	$(CC) $(BANK) -o $@ $<

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
	$(DEV)/worldmap.sh

strings.c strings.h: strings.ini stringmap.txt specialchars.txt
	$(DEV)/ini2c.py $^ -o $@

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

clean:
	rm -f pix/*_gb.png level.c strings.c strings.h pix/pix.h music/songs.h
	find . -maxdepth 2 -type f -regex '.*.\(gb\|o\|map\|lst\|sym\|rel\|ihx\|lk\|noi\|asm\|adb\|cdb\|bi4\|pal\|2bpp\|1bpp\|xbpp\|tilemap\)' -delete
	find . -maxdepth 2 -type f -regex '.*_\(map\|data\|pal\|tmap\)\.c' -delete
	find . -maxdepth 2 -type f -regex '.*_\(gb\|mono\)\.png' -delete
	$(MAKE) -C $(DEV)/gbdk-music clean
	$(MAKE) -C $(DEV)/png2gb clean

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

.PHONY: spaceleft
spaceleft: build
	dev/romusage/bin/romusage $(ROM).noi

FORCE: