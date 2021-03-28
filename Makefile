# disable builtin rules
.SUFFIXES: 

# for not installed gbdk and sdcc
GBDKBIN=
SDCCBIN=

# globally installed
LCC=$(GBDKBIN)lcc -v
# use --sdccbindir= for a custom sdcc
ifneq ($(SDCCBIN),)
LCC+= --sdccbindir=$(SDCCBIN)
endif

CPP=$(LCC) -Wf-E
CPPFLAGS=
CC=$(LCC)
CFLAGS=-Wf--opt-code-size -Wf--max-allocs-per-node50000
AR=$(SDCCBIN)sdar
ARFLAGS=
AS=$(LCC)
ASFLAGS=-c
LD=$(LCC)
LDFLAGS=-Wm-yn"DESSERTONAVEGI" -Wm-yc -Wm-yt0x03 -Wm-ya1 -Wl-j -Wm-yS

BUILDDIR=build/
BINDIR=bin/
VPATH=src:$(BUILDDIR)

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
# 0: no debug
# 1: all labels in .sym
# 2: .cdb file
# 3: data in extra rom (not runnable)
# 4: disable optimizations
# 9: show complexity of compiled functions (and do 1)
ifeq ($(ROMDEBUG), 0)
BANK=
else
CFLAGS+= -Wf--fverbose-asm
ASFLAGS+= -Wa-a
ifeq ($(ROMDEBUG), 9)
CFLAGS+= -Wf--cyclomatic
else
ifneq ($(ROMDEBUG), 1)
CFLAGS+= -debug
LDFLAGS+= -debug
ifneq ($(ROMDEBUG), 2)
BANK= -Wf-bo3
LDFLAGS+= -Wm-yoA
ifneq ($(ROMDEBUG), 3)
CFLAGS+= -Wf--no-peep -Wf--nolospre -Wf--noloopreverse -Wf--noinduction -Wf--noinvariant
endif
endif
endif
endif
endif

LEVELTMX=$(wildcard level/lvl_*.tmx)
LEVEL=${subst level/,$(BUILDDIR),$(LEVELTMX:.tmx=_tmap.c)}
MUSIC=dev/gbdk-music/music/the_journey_begins.c music/cosmicgem_voadi.c
PIX=$(addprefix $(BUILDDIR),$(addsuffix _data.c,overworld_a_gbc overworld_b_gbc inside_wood_house overworld_anim_gbc overworld_cave characters win_gbc_pb16 modular_characters body_move_a_gbc_pb16 body_move_b_gbc_pb16 body_idle_gbc_pb16 body_stand_gbc_pb16 items_gbc_pb16))

ROM=doavi

########################################################

.PHONY: build run spaceleft statistics clean gbonline

build: $(BUILDDIR) $(BINDIR) $(BINDIR)$(ROM).gb

%/:
	mkdir -p $@

$(BINDIR)$(ROM).gb: $(BUILDDIR)main.rel $(BUILDDIR)hud.rel ./dev/gbdk-music/music.rel $(BUILDDIR)map.rel $(BUILDDIR)logic.rel $(BUILDDIR)undice.rel $(BUILDDIR)unpb16.rel $(BUILDDIR)unlz3.rel $(BUILDDIR)strings.rel $(BUILDDIR)level.rel $(BUILDDIR)songs.rel $(BUILDDIR)pix.rel
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILDDIR)%.asm: %.c
	$(CC) $(CFLAGS) -S -o $@ $^

$(BUILDDIR)main.asm: main.c pix.h strings.h
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILDDIR)logic.asm: logic.c level.h strings.h
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILDDIR)hud.asm: hud.c pix.h
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILDDIR)map.asm: map.c pix.h songs.h
	$(CC) $(CFLAGS) -S -o $@ $<

$(BUILDDIR)strings.asm: strings.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

$(BUILDDIR)level.asm: level.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

$(BUILDDIR)songs.asm: music/songs.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $^

$(BUILDDIR)pix.asm: pix/pix.c $(BUILDDIR)overworld_a_gbc_pb16_data.c $(BUILDDIR)overworld_b_gbc_pb16_data.c $(BUILDDIR)overworld_cave_pb16_data.c $(BUILDDIR)inside_wood_house_pb16_data.c $(BUILDDIR)modular_characters_pb16_data.c $(BUILDDIR)dialog_mouths_lz3_data.c $(BUILDDIR)dialog_photos_lz3_data.c $(PIX) $(BUILDDIR)inside_wood_house_map.c $(BUILDDIR)overworld_a_gbc_map.c $(BUILDDIR)overworld_b_gbc_map.c $(BUILDDIR)overworld_cave_map.c $(BUILDDIR)modular_characters_map.c $(BUILDDIR)modular_characters_map.c $(BUILDDIR)overworld_anim_gbc_map.c $(BUILDDIR)overworld_a_gbc_pal.c $(BUILDDIR)overworld_b_gbc_pal.c $(BUILDDIR)characters_pal.c $(BUILDDIR)hud_pal.c
	$(CC) $(CFLAGS) $(BANK) -S -o $@ $<

# generated
$(BUILDDIR)%.rel: %.asm
	$(AS) $(ASFLAGS) -o $@ $^

# handwritten
$(BUILDDIR)%.rel: %.s
	$(AS) $(ASFLAGS) -o $@ $^

$(BUILDDIR)pix.h: pix/pix.c $(BUILDDIR)pix.rel
	$(c2h) $< > $@

$(BUILDDIR)songs.h: music/songs.c $(BUILDDIR)songs.rel
	grep "music.h" music/cosmicgem_voadi.c > $@
	$(c2h) $< >> $@

$(BUILDDIR)dialog_photos.2bpp: pix/dialog_photos.png
	$(pngconvert) -cno --width 4 --height 4 -u yes $^ -o $@

$(BUILDDIR)modular_characters.2bpp $(BUILDDIR)modular_characters.pal $(BUILDDIR)modular_characters.tilemap: pix/body_move_a_gbc.png pix/body_idle_gbc.png pix/body_stand_gbc.png pix/body_gbc.png  pix/body_ghost_gbc.png $(addprefix pix/head_,$(addsuffix _gbc.png,candy male0 ghost robot0 robot1 female0 male2 rachel male6 male1 male3))
	$(pngconvert) -cno -flip $^ -o $@

$(BUILDDIR)characters_data.c : pix/angry_toast_gbc.png pix/muffin_gbc.png  pix/ghost_gbc.png
	$(pngconvert) --width 2 --height 2 -u yes $^ -o $@

$(BUILDDIR)win_gbc.2bpp $(BUILDDIR)win_gbc.pal $(BUILDDIR)win_gbc.tilemap: pix/win_gbc.png pix/squont8ng_gbc.png
	$(pngconvert) -cno -u yes $^ -o $@

$(BUILDDIR)overworld_a_gbc.2bpp $(BUILDDIR)overworld_a_gbc.pal $(BUILDDIR)overworld_a_gbc.tilemap: pix/overworld_a_gbc.png pix/overworld_path_gbc.png pix/house_wood_round.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

$(BUILDDIR)overworld_b_gbc.2bpp $(BUILDDIR)overworld_b_gbc.pal $(BUILDDIR)overworld_b_gbc.tilemap: pix/overworld_b_gbc.png pix/sand_bottle.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

$(BUILDDIR)inside_wood_house.2bpp $(BUILDDIR)inside_wood_house.pal $(BUILDDIR)inside_wood_house.tilemap: pix/inside_wood_house.png pix/carpet_gbc.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

$(BUILDDIR)overworld_cave.2bpp $(BUILDDIR)overworld_cave.pal $(BUILDDIR)overworld_cave.tilemap: pix/overworld_cave.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 --limit 128

$(BUILDDIR)%_anim_gbc.2bpp $(BUILDDIR)%_anim_gbc.pal $(BUILDDIR)%_anim_gbc.tilemap: pix/%_anim_gbc.png
	$(pngconvert) -cno $^ -o $@ --width 2 --height 2 -u yes

$(BUILDDIR)hud_pal.c: pix/win_gbc.png
	$(pngconvert) $^ -o$@

$(BUILDDIR)%_data.c: $(BUILDDIR)%.2bpp
	$(bin2c) $^ $@ "png2gb.py and xxd" $$(($$(stat --printf="%s" $$(echo $^ |sed 's/_\(rle\|pb16\|pb8\|lz3\)//g'))/16))

$(BUILDDIR)%_map.c: $(BUILDDIR)%.tilemap
	$(bin2c) $^ $@ "png2gb.py and xxd"

$(BUILDDIR)%_pal.c: $(BUILDDIR)%.pal
	$(bin2c) $^ $@ "png2gb.py and xxd"

$(BUILDDIR)%_tmap.c: level/%.tmx
ifeq ($(COMPRESS),1)
	$(tmxconvert) --compress 1 -o$@ $^
else
	$(tmxconvert) -o$@ $^
endif

$(BUILDDIR)level.c: $(LEVEL)
	./dev/worldmap.sh $@ $^

$(BUILDDIR)strings.c $(BUILDDIR)strings.h: strings.ini stringmap.txt specialchars.txt
	./dev/ini2c.py $^ -o $@

$(BUILDDIR)%.2bpp $(BUILDDIR)%.pal $(BUILDDIR)%.tilemap: pix/%.png
	$(pngconvert) -cno $< -o $@

$(BUILDDIR)%_lz3.2bpp: $(BUILDDIR)%.2bpp
	$(lz3) $< $@

$(BUILDDIR)%_pb16.2bpp: $(BUILDDIR)%.2bpp
	$(pb16) $^ $@

$(BUILDDIR)%.pal: pix/%_gbc.png
	$(gbc2gb) $^

$(BUILDDIR)%.pal: pix/%.png
	$(gbc2gb) $^

$(BUILDDIR)%_gb.png: pix/%_gbc.png
	$(gbc2gb) $^

$(BUILDDIR)%_gb.png: pix/%.png
	$(gbc2gb) $^

gbonline:  $(BINDIR)$(ROM).gb ./dev/GameBoy-Online/
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
	$(EMU) $(BINDIR)$(ROM).gb

clean:
	find bin/ -type f -regex '.*.\(ihx\|map\|noi\|cdb\)' -delete
	find $(BUILDDIR) -type f -regex '.*.\(rel\|c\|h\|asm\|2bpp\|tilemap\|pal\|adb\)' -delete
	$(MAKE) -C ./dev/gbdk-music clean
	$(MAKE) -C ./dev/png2gb clean

spaceleft: build
	dev/romusage/bin/romusage $(BINDIR)$(ROM).noi -g -E

statistics: build
	test -e $(BINDIR)$(ROM).cdb && dev/romusage/bin/romusage $(BINDIR)$(ROM).cdb -g
	dev/romusage/bin/romusage $(BINDIR)$(ROM).noi -G -E -sH -a

FORCE: