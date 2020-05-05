#include <string.h>

#include "map.h"
#include "main.h"

#include "pix/inside_wood_house_data.c"
#include "pix/overworld_a_gbc_data.c"
#include "pix/overworld_b_gbc_data.c"
#include "pix/overworld_cave_data.c"

#include "pix/inside_wood_house_map.c"
#include "pix/overworld_a_gbc_map.c"
#include "pix/overworld_b_gbc_map.c"
#include "pix/overworld_cave_map.c"

#include "pix/inside_wood_house_pal.c"
#include "pix/overworld_a_gbc_pal.c"
#include "pix/overworld_b_gbc_pal.c"
#include "pix/overworld_cave_pal.c"

#include "dev/png2gb/csrc/decompress.h"

const unsigned char *current_map;
const unsigned char *loaded_map;
// always the same size
UINT8 decompressed_background[80];

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
    UINT8 hi = 0;
    UINT8 lo = 0;
    for(UINT8 i = 0; i < 80; ++i){
        if(counter != 0){
            decompressed_background[i] = byte;
            --counter;
        }else{
            lo = compressed_map[c] & 0xF;
            hi = compressed_map[c] & 0xF;
            if(bytepart == 0){
                hi = (compressed_map[c] >> 4) & 0xF;
            }else{
                lo = (compressed_map[c+1] >> 4) & 0xF;
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
                bytepart = (bytepart + 1) % 2;
                if(bytepart == 0)
                    ++c;
            }else if((hi & 0x4) == 0){
                //other tiles
                decompressed_background[i] = (hi & 0x3)<<4 | lo;
                ++c;
            }else{
                //special
                decompressed_background[i] = 2;
                // 4x grass
                if(lo == 0 && hi == 0xC){
                    counter = 3;
                    byte = 2;
                }
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
    set_bkg_tiles(x * 2 + 1, y * 2 + 1, 2, 2, tiles);
    VBK_REG = 0;
}

void incject_map(UINT8 x, UINT8 y, UINT16 index) {
    unsigned char tiles[4];
    index *= 4;
    tiles[0] = SHEET_START + current_map[index];
    tiles[1] = SHEET_START + current_map[index + 2];
    tiles[2] = SHEET_START + current_map[index + 1];
    tiles[3] = SHEET_START + current_map[index + 3];
    set_bkg_tiles(x * 2 + 1, y * 2 + 1, 2, 2, tiles);
}

void load_map(const UINT8 background[]) {
    UINT8 y;
    UINT8 x;
    UINT16 index;
    // tmx
    UINT16 tile;
    // loaded spritesheet
    UINT8 palette;
    unsigned char tiles[4];

    DISPLAY_OFF;
    // load spritesheet
    if (sg->level_y == 4) {//0b0100
        if(current_map != overworld_b_gbc_map){
            current_map = overworld_b_gbc_map;
            loaded_map = overworld_b_gbc_map;
            set_bkg_data_rle(SHEET_START, overworld_b_gbc_data_length,
                        overworld_b_gbc_data, 0);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
            BGP_REG = 0xE4; // 11100100
        }
    } else if (sg->level_y == 5) {//0b0101
        if(current_map != inside_wood_house_map){
            current_map = inside_wood_house_map;
            loaded_map = inside_wood_house_map;
            set_bkg_data_rle(SHEET_START, inside_wood_house_data_length,
                        inside_wood_house_data, 0);
            set_bkg_palette(0, 6, inside_wood_house_pal[0]);
            BGP_REG = 0xE4; // 11100100
        }
    } else if (sg->level_y > 5) {
        if(current_map != overworld_cave_map){
            current_map = overworld_cave_map;
            loaded_map = overworld_cave_map;
            set_bkg_data_rle(SHEET_START, overworld_cave_data_length,
                        overworld_cave_data, 0);
            set_bkg_palette(0, 6, overworld_cave_pal[0]);
            BGP_REG = 0xE4; // 11100100
        }
    } else {
        if(current_map != overworld_a_gbc_map){
            current_map = overworld_a_gbc_map;
            loaded_map = overworld_a_gbc_map;
            set_bkg_data_rle(SHEET_START, overworld_a_gbc_data_length,
                        overworld_a_gbc_data, 0);
            set_bkg_palette(0, 6, overworld_a_gbc_pal[0]);
            BGP_REG = 0xE1;
        }
    }

    for (y = 0; y < HEIGHT; ++y) {
        UINT8 tmp1 = y * WIDTH;
        UINT8 tmp2 = y * 2 + 1;
        for (x = 0; x < WIDTH; ++x) {
            UINT8 tmpx = x * 2 + 1;
            // load background
            tile = background[tmp1 + x] - 2;
            index = tile * 4;
            // set color (GBC only)
            VBK_REG = 1;
            // each row has own palette
            palette = tile / (SPRITEWIDTH / 2);
            if (current_map == overworld_a_gbc_map && palette == 3 &&
                (tile % (SPRITEWIDTH / 2)) >= 4) {
                // last row has two palletes
                palette = 4;
            }
            // houses extension
            if (current_map == overworld_a_gbc_map && palette > 3) {
                palette = 2;
            }
            // inside house
            if (current_map == inside_wood_house_map) {
                palette = 2;
            }
            tiles[0] = tiles[1] = tiles[2] = tiles[3] = palette;
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            tiles[0] = SHEET_START + current_map[index];
            tiles[1] = SHEET_START + current_map[index + 2];
            tiles[2] = SHEET_START + current_map[index + 1];
            tiles[3] = SHEET_START + current_map[index + 3];
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
        }
    }

    // map scripting
    if (!(sg->collectable & 0x1) && sg->level_x == 1 && sg->level_y == 0) {
        incject_map(2, 2, 29);
    }
    DISPLAY_ON;

    if((sg->collectable & (1<<2)) && sg->level_x == 0 && sg->level_y == 0){
        // spawn ghost
        sg->character[0].x = 4;
        sg->character[0].y = 2;
        sg->character[0].sprite = 2;
        sg->character[0].direction = 0;
        sg->character[0].palette = 3;
        sg->character[0].offset_x = 0;
        sg->character[0].offset_y = 0;

        render_character(&(sg->character[0]));
    }
}