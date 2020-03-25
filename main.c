// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "pix/overworld_gb_data.c"
#include "pix/overworld_gb_map.c"
#include "pix/demo_tmap.c"

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HIGHT (9)
#define WIDTH (10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH (34)
#define TRANSPARENT (RGB(31, 0, 31))

unsigned int used_sprites;

void load_map(const unsigned int background[], const unsigned int sprites[]) {
	int y;
	int x;
	int index;
	int i;
	// tmx
	unsigned int tile;
	// loaded spritesheet
	unsigned int sprite_y;
	unsigned int sprite_x;
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
			tiles[0] = overworld_gb_map[index];
			tiles[1] = overworld_gb_map[index + 1];
			tiles[2] = overworld_gb_map[index + SPRITEWIDTH];
			tiles[3] = overworld_gb_map[index + 1 + SPRITEWIDTH];
			set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);

			// load sprites
			tile = sprites[(y * WIDTH) + x];
			if(tile != 0){
				tile-=2;
				sprite_x = tile % (SPRITEWIDTH/2);
				sprite_y = tile / (SPRITEWIDTH/2);
				index = (sprite_y * 2 * SPRITEWIDTH) + (sprite_x * 2);
				set_sprite_tile(used_sprites, overworld_gb_map[index]);
				move_sprite(used_sprites, 8 + x*16, 16 + y*16);
				set_sprite_prop(used_sprites, sprite_y);
				set_sprite_tile(used_sprites+1, overworld_gb_map[index + 1]);
				move_sprite(used_sprites+1, 8 + x*16 + 8, 16 + y*16);
				set_sprite_prop(used_sprites+1, sprite_y);
				set_sprite_tile(used_sprites+2, overworld_gb_map[index + SPRITEWIDTH]);
				move_sprite(used_sprites+2, 8 + x*16, 16 + y*16 + 8);
				set_sprite_prop(used_sprites+2, sprite_y);
				set_sprite_tile(used_sprites+3, overworld_gb_map[index + 1 + SPRITEWIDTH]);
				move_sprite(used_sprites+3, 8 + x*16 + 8, 16 + y*16 + 8);
				set_sprite_prop(used_sprites+3, sprite_y);
				used_sprites+=4;
			}
		}
	}
}

// right to left
UWORD bkgPalette[][] = {{
	(RGB(18, 7, 0)), TRANSPARENT, (RGB(3, 14, 0)), (RGB(0, 0, 0))
},{
	(RGB(28, 16, 0)), TRANSPARENT, (RGB(21, 3, 1)), (RGB(0, 0, 0))
},{
	(RGB(18, 18, 18)), TRANSPARENT, (RGB(10, 10, 10)), (RGB(0, 0, 0))
},{
	(RGB(25, 25, 12)), (RGB(12, 25, 0)), TRANSPARENT, (RGB(3, 14, 0))
},{
	(RGB(28, 16, 0)), (RGB(18, 7, 0)), TRANSPARENT, (RGB(0, 0, 0))
}};

void main() {
	NR52_REG = 0x80; // enable sound
	NR50_REG = 0x77; // full volume
	NR51_REG = 0xFF; // all channels
	int i;
	SPRITES_8x8;
	used_sprites = 0;

	cgb_compatibility();
	set_bkg_palette(0, 5, bkgPalette[0]);
	set_sprite_palette(0, 5, bkgPalette[0]);

	//load tileset
	set_bkg_data(0,144,overworld_gb_data);
	set_sprite_data(0,144,overworld_gb_data);
	load_map(demo_tmap_background, demo_tmap_sprites);
	
	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;
}
