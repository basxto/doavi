// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

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
#define WIN_START (0)

UINT8 used_sprites;

void init_hud(){
	unsigned char tiles[1];
	UINT8 x;
	UINT8 y;
	VBK_REG=1;
	tiles[0] = 2;
	for(x = 0; x < 20; ++x){
		for(y = 0; y < 4; ++y){
			set_win_tiles(x,y,1,1,tiles);
		}
	}
	VBK_REG=0;
	tiles[0] = WIN_START+24;
	for(x = 0; x < 20; ++x){
		for(y = 0; y < 4; ++y){
			set_win_tiles(x,y,1,1,tiles);
		}
	}
}

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
	UINT8 buffer[16];
    for (i = 0; i != 16; i++) {
        buffer[i] = WIN_START + ' ';
    }
    for (i = 0; i != length; i++) {
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        if (str[i] > 0x20 && str[i] < 0x60) {
            buffer[i] = WIN_START + (str[i]);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            buffer[i] = WIN_START + (str[i] - 0x20);
        } else {
            // everything else, including space, becomes a space
            buffer[i] = WIN_START + ' ';
        }
    }
    set_win_tiles(x, y, length, 1, buffer);
}

// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num) {
    char buffer[] = "000";
    if (length == 0) {
        return;
    }
    if (length & (~3)) { // >3
        length = 3;
    }
    buffer[2] = '0' + (num % 10);
    num /= 10;
    buffer[1] = '0' + (num % 10);
    num /= 10;
    buffer[0] = '0' + (num % 10);
    num /= 10;
    write_line(x, y, length, buffer + (3 - length));
}

void draw_hud(const UINT8 lives, const UINT8 toiletpaper){
	UINT8 i;
	unsigned char tiles[2];
	tiles[0] = WIN_START+9;
	set_win_tiles(2,1,1,1,tiles);
	tiles[0] = WIN_START+8;
	set_win_tiles(3,1,1,1,tiles);
	tiles[0] = WIN_START+12;
	tiles[1] = WIN_START+13;
	set_win_tiles(0,0,2,1,tiles);
	tiles[0] = WIN_START+14;
	tiles[1] = WIN_START+15;
	set_win_tiles(0,1,2,1,tiles);
	for(i = 0; i < 5; ++i){
		if(i >= lives){
			tiles[0] = WIN_START+11;
		}else{
			tiles[0] = WIN_START+10;
		}
		set_win_tiles(2+i,0,1,1,tiles);
	}
	write_num(4,1,3,toiletpaper);
	move_win(7,16*8);
}

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

void main() {
	NR52_REG = 0x80; // enable sound
	NR50_REG = 0x77; // full volume
	NR51_REG = 0xFF; // all channels
	SPRITES_8x8;
	used_sprites = 0;

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
}
