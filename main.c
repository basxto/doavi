// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "hud.h"
#include "music.h"

#include "pix/overworld_gb_data.c"
#include "pix/overworld_gb_map.c"
#include "pix/win_gb_data.c"
#include "pix/demo_tmap.c"

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HIGHT (8)
#define WIDTH (10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH (34)
#define TRANSPARENT (RGB(12, 25, 0))

#define SHEET_START (97)

UINT8 used_sprites;
UINT8 counter;

void load_map(const unsigned int background[], const unsigned int sprites[]) {
	UINT8 y;
	UINT8 x;
	int index;
	UINT8 i;
	// tmx
	UINT16 tile;
	// loaded spritesheet
	UINT8 sprite_y;
	UINT8 sprite_x;
	unsigned char tiles[4];

	// reset all sprites used_sprites
	for(i = 0; i < used_sprites; ++i)
		move_sprite(used_sprites, 0, 0);

	for(y = 0; y < HIGHT; ++y){
		for(x = 0; x < WIDTH; ++x){
			// load background
			tile = background[(y * WIDTH) + x] - 2;
			sprite_x = tile % (SPRITEWIDTH/2);
			sprite_y = tile / (SPRITEWIDTH/2);
			index = (sprite_y * 2 * SPRITEWIDTH) + (sprite_x * 2);
			// set color (GBC only)
			VBK_REG=1;
			// each row has own palette
			tiles[0] = tiles[1] = tiles[2] = tiles[3] = sprite_y;
			set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);
			VBK_REG=0;
			// set tiles
			tiles[0] = SHEET_START + overworld_gb_map[index];
			tiles[1] = SHEET_START + overworld_gb_map[index + 1];
			tiles[2] = SHEET_START + overworld_gb_map[index + SPRITEWIDTH];
			tiles[3] = SHEET_START + overworld_gb_map[index + 1 + SPRITEWIDTH];
			set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);

			// load sprites
			tile = sprites[(y * WIDTH) + x];
			if(tile != 0){
				tile-=2;
				sprite_x = tile % (SPRITEWIDTH/2);
				sprite_y = tile / (SPRITEWIDTH/2);
				index = (sprite_y * 2 * SPRITEWIDTH) + (sprite_x * 2);
				set_sprite_tile(used_sprites, SHEET_START + overworld_gb_map[index]);
				move_sprite(used_sprites, 8 + x*16, 16 + y*16);
				set_sprite_prop(used_sprites, sprite_y);
				set_sprite_tile(used_sprites+1, SHEET_START + overworld_gb_map[index + 1]);
				move_sprite(used_sprites+1, 8 + x*16 + 8, 16 + y*16);
				set_sprite_prop(used_sprites+1, sprite_y);
				set_sprite_tile(used_sprites+2, SHEET_START + overworld_gb_map[index + SPRITEWIDTH]);
				move_sprite(used_sprites+2, 8 + x*16, 16 + y*16 + 8);
				set_sprite_prop(used_sprites+2, sprite_y);
				set_sprite_tile(used_sprites+3, SHEET_START + overworld_gb_map[index + 1 + SPRITEWIDTH]);
				move_sprite(used_sprites+3, 8 + x*16 + 8, 16 + y*16 + 8);
				set_sprite_prop(used_sprites+3, sprite_y);
				used_sprites+=4;
			}
		}
	}
}

// right to left
UWORD bkgPalette[][] = {{
	TRANSPARENT, (RGB(25, 25, 12)), (RGB(12, 25, 0)), (RGB(3, 14, 0))
},{
	TRANSPARENT, (RGB(18, 7, 0)), (RGB(3, 14, 0)), (RGB(0, 0, 0))
},{
	TRANSPARENT, (RGB(18, 7, 0)), (RGB(28, 16, 0)), (RGB(0, 0, 0))
},{
	TRANSPARENT, (RGB(18, 18, 18)), (RGB(10, 10, 10)), (RGB(0, 0, 0))
},{
	TRANSPARENT, (RGB(28, 16, 0)), (RGB(21, 3, 1)), (RGB(0, 0, 0))
}};

void timer_isr(){
	if(counter == 0){
		tick_music();
	}
	counter++;
	counter %= 4;
}

void main() {
	NR52_REG = 0x80; // enable sound
	NR50_REG = 0x77; // full volume
	NR51_REG = 0xFF; // all channels
	SPRITES_8x8;
	used_sprites = 0;
	counter = 0;

	cgb_compatibility();
	set_bkg_palette(0, 5, bkgPalette[0]);
	set_sprite_palette(0, 5, bkgPalette[0]);

	//load tileset
	set_bkg_data(SHEET_START,136,overworld_gb_data);
	set_sprite_data(SHEET_START,136,overworld_gb_data);
	set_win_data(WIN_START,96,win_gb_data);
	load_map(demo_tmap_background, demo_tmap_sprites);
	init_hud();
	draw_hud(2, 42);
	
	SHOW_BKG;
	SHOW_WIN;
	SHOW_SPRITES;
	DISPLAY_ON;

	init_music();
	// configure interrupt
	TIMA_REG = TMA_REG = 0x1A;
	TAC_REG = 0x4 | 0x0;//4096 Hz
	// enable timer interrupt
	disable_interrupts();
	add_TIM(timer_isr);
	enable_interrupts();
	set_interrupts(TIM_IFLAG);
}
