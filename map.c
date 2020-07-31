#include <string.h>

#include "map.h"
#include "main.h"

#include "pix/pix.h"

#include "music/songs.h"

#include "dev/png2gb/csrc/decompress.h"
#include "unpb16.h"

#define WIN_START 0

const unsigned char *current_map;
const unsigned char *loaded_map;
// always the same size
UINT8 decompressed_background[80];
UINT8 decompressed_tileset[128*16];
// draw next map to a different part of screen
static _Bool background_shifted;

// from main.c
extern UINT8 current_collision[10];

#ifdef COMPRESS

// 0XXX short notation
// 10XX XXXX other tiles (long notation)
// 11XX XXXX special tiles
// compressed with dev/tmx2c.py --compress=1
const unsigned char * decompress(const UINT8 *compressed_map){
    UINT8 c = 0;
    UINT8 bytepart = 0;
    UINT8 byte = 0;
    UINT8 counter = 0;
    for(UINT8 i = 0; i < 80; ++i){
        if(counter != 0){
            decompressed_background[i] = byte;
            --counter;
        }else{
            UINT8 lo = compressed_map[c] & 0xF;
            UINT8 hi = compressed_map[c] & 0xF;
            if(bytepart == 0){
                hi = (compressed_map[c] >> 4);
            }else{
                lo = (compressed_map[c+1] >> 4);
            }
            if((hi & 0x8) == 0){
                // short notation
                if(hi < 5){//<0b101 is 0b100 0b011 0b010 0b001
                    decompressed_background[i] = hi+2;
                }else{
                    hi*=2;
                    decompressed_background[i++] = hi;
                    decompressed_background[i] = hi + 1;
                }
                bytepart = $(bytepart + 1) % $(2);
                if(bytepart == 0)
                    ++c;
            }else if((hi & 0x4) == 0){
                //other tiles
                decompressed_background[i] = $(hi & 0x3)<<4 | lo;
                ++c;
            }else{
                //special
                // treat all as four times
                byte = (((hi%4)<<4)|lo)+2;
                decompressed_background[i] = byte;
                counter = 3;
                ++c;
            }
        }

    }
    return decompressed_background;
}
#else
// make a copy in RAM, so it can be changed later
const unsigned char * decompress(const UINT8 *compressed_map){
    memcpy(decompressed_background, compressed_map, 80);
    return decompressed_background;
}
#endif

void incject_map_palette(const UINT8 x, const UINT8 y, const UINT8 index) {
    unsigned char tiles[4] = {index, index, index, index};
    VBK_REG = 1;
    set_bkg_tiles(x * $(2) + 1, y * $(2) + (background_shifted ? 0 : 0x10), 2, 2, tiles);
    VBK_REG = 0;
}

void incject_map(UINT8 x, UINT8 y, UINT16 index) {
    unsigned char tiles[4];
    index *= 4;
    UINT8 j = 0;// goes 0 2 1 3
    for(UINT8 i = 0; i < 4; ++i){
        tiles[i] = SHEET_START + current_map[index + j];
        if(j==2)
            --j;
        else
            j+=2;
    }
    set_bkg_tiles(x * $(2) + 1, y * $(2) + (background_shifted ? 0 : 0x10), 2, 2, tiles);
}

void incject_collision(UINT8 x, UINT8 y, _Bool enable) {
    UINT8 index = (y) * WIDTH + (x);
    if(enable)
        current_collision[index / $(8)] |= (1 << (index % $(8)));
    else
        current_collision[index / $(8)] &= ~(1 << (index % $(8)));
}

void load_map(const UINT8 background[]) {
    unsigned char *next_map;
    UINT8 y;
    UINT8 x;
    UINT16 index;
    // tmx
    UINT16 tile;
    // loaded spritesheet
    UINT8 palette;
    unsigned char tiles[4];

    background_shifted = !background_shifted;
    if (level_y == 4) {//0b0100
        next_map = overworld_b_gbc_map;
    } else if (level_y == 5) {//0b0101
        next_map = inside_wood_house_map;
    } else if (level_y > 5) {
        next_map = overworld_cave_map;
    } else {
        next_map = overworld_a_gbc_map;
    }

    for (y = 0; y < HEIGHT; ++y) {
        UINT8 tmp1 = y * WIDTH;
        UINT8 tmp2 = y * 2 + (background_shifted ? 0 : 0x10);
        for (x = 0; x < WIDTH; ++x) {
            UINT8 tmpx = x * $(2) + $(1);
            // load background
            tile = background[tmp1 + x] - 2;
            if(current_chest != 0 && !(chest & current_chest) && (tile == 29 || tile == 18))
                --tile;
            if(current_flame != 0 && (flame & current_flame) && tile == 30)
                ++tile;
            index = tile * 4;
            // set color (GBC only)
            VBK_REG = 1;
            // each row has own palette
            palette = tile / (SPRITEWIDTH / 2);
            if (next_map == overworld_a_gbc_map && palette == 3 &&
                (tile % (SPRITEWIDTH / 2)) >= 4) {
                // last row has two palletes
                palette = 4;
            }
            if(palette > 3)
                if (next_map == overworld_a_gbc_map) {
                    // houses extension
                    palette = 2;
                }
                else if (next_map == overworld_b_gbc_map) {
                    // bottle
                    palette = 1;
                }
            // inside house
            if (next_map == inside_wood_house_map) {
                palette = 2;
                if(tile == 7 || tile >= 32)
                    palette = 4;//carpet and flame
                if(tile == 14)
                    palette = 1;//plant
            }
            if (next_map == overworld_cave_map) {
                palette = 2;
            }
            tiles[0] = tiles[1] = tiles[2] = tiles[3] = palette;
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            UINT8 j = 0;// goes 0 2 1 3
            for(UINT8 i = 0; i < 4; ++i){
                tiles[i] = SHEET_START + next_map[index + j];
                if(j==2)
                    --j;
                else
                    j+=2;
            }
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
        }
    }
    // change map back when time jumping
    if(level_x == 6){
        if(level_y == 0)
            level_x = 0;
        if(level_y == 1)
            level_x = 4;
    }
    DISPLAY_OFF;
    BGP_REG = 0xE4; // 11100100
    // load spritesheet
    if (level_y == 4) {//0b0100
        if(current_map != overworld_b_gbc_map){
            init_music(&cosmicgem_voadi);
            current_map = overworld_b_gbc_map;
            loaded_map = overworld_b_gbc_map;
            pb16_unpack_bkg_data(SHEET_START, overworld_b_gbc_data_length, decompressed_tileset, overworld_b_gbc_data);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else if (level_y == 5) {//0b0101
        if(current_map != inside_wood_house_map){
            current_map = inside_wood_house_map;
            loaded_map = inside_wood_house_map;
            pb16_unpack_bkg_data(SHEET_START, inside_wood_house_data_length, decompressed_tileset, inside_wood_house_data);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else if (level_y > 5) {
        if(current_map != overworld_cave_map){
            current_map = overworld_cave_map;
            loaded_map = overworld_cave_map;
            pb16_unpack_bkg_data(SHEET_START, overworld_cave_data_length, decompressed_tileset, overworld_cave_data);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else {
        if(current_map != overworld_a_gbc_map){
            if(current_map == overworld_b_gbc_map || current_map == 0)
                init_music(&the_journey_begins);
            current_map = overworld_a_gbc_map;
            loaded_map = overworld_a_gbc_map;
            pb16_unpack_bkg_data(SHEET_START, overworld_a_gbc_data_length, decompressed_tileset, overworld_a_gbc_data);
            set_bkg_palette(0, 6, overworld_a_gbc_pal[0]);
        }
        BGP_REG = 0xE1;
    }

    // map scripting

    if(cheat && level_y == 0){
        if(level_x == 1){
            incject_collision(9, 2, 0);
        }
        if(level_x == 3){
            incject_collision(4, 1, 0);
            incject_collision(6, 1, 0);
        }
        if(level_x == 5){
            incject_collision(6, 1, 0);
        }
    }

    // only show ghost in present
    if(IS_PRGRS_TIME(0) && IS_PRGRS_GHOST(1) && level_x == 0 && level_y == 0){
        // spawn ghost
        character[1].x = 4;
        character[1].y = 2;
        character[1].sprite = 2;
        character[1].direction = 7<<2;//ghost bottom
        character[1].palette = 3<<4 | 3;
    }
    else if(level_x == 4 && level_y == 0){
        // spawn timetravel robot
        character[1].x = 4;
        character[1].y = 2;
        character[1].sprite = 4;
        character[1].palette = 1<<4 | 1;
        // spawn broken timetravel robot
        character[2].x = 8;
        character[2].y = 6;
        character[2].sprite = 3;
        character[2].palette = 1<<4 | 1;
    }
    else if(level_x == 4 && level_y == 5){
        // spawn rachel
        character[1].x = 4;
        character[1].y = 3;
        character[1].sprite = 7;
        character[1].palette = 1<<4 | 1;
    } else if(IS_PRGRS_TIME(1)) {
        if(level_y == 5){
            switch(level_x){
              case 0:
                character[1].x = 2;
                character[1].y = 3;
                character[1].sprite = 6;
                character[1].palette = 1<<4 | 1;
                break;
              case 1:
                character[1].x = 3;
                character[1].y = 5;
                character[1].sprite = 10;
                character[1].palette = 1<<4 | 1;
                break;
              case 2:
                character[1].x = 4;
                character[1].y = 3;
                character[1].sprite = 1;
                character[1].palette = 1<<4 | 1;
                break;
              case 3:
                character[1].x = 7;
                character[1].y = 6;
                character[1].sprite = 9;
                character[1].palette = 1<<4 | 1;
                character[2].x = 6;
                character[2].y = 4;
                character[2].sprite = 5;
                character[2].direction += 2;
                character[2].palette = 1<<4 | 1;
                break;
            }
        }else if(level_x == 3 && level_y == 1){
            character[1].x = 4;
            character[1].y = 3;
            character[1].sprite = 8;
            character[1].palette = 1<<4 | 1;
        }
    }

    if(background_shifted == 0){
        move_bkg(8, 0x10*8);
    }else{
        move_bkg(8, 0);
    }
    VBK_REG = 1;
    tiles[0] = 1;
    y = (background_shifted ? 16 : 0);
    // make it all black
    for (x = 1; x <= 20; ++x) {
        set_bkg_tiles(x, y+0, 1, 1, tiles);
        set_bkg_tiles(x, y+1, 1, 1, tiles);
        set_bkg_tiles(x, (y+15)%32, 1, 1, tiles);
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + 6;
    for (x = 1; x <= 20; ++x) {
        set_bkg_tiles(x, y+0, 1, 1, tiles);
        set_bkg_tiles(x, y+1, 1, 1, tiles);
        set_bkg_tiles(x, (y+15)%32, 1, 1, tiles);
    }
    // finally move character
    render_character(0);
    render_character(1);
    render_character(2);
    DISPLAY_ON;
}