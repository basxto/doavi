#include "map.h"
#include "main.h"

#include "pix/inside_wood_house_data.c"
#include "pix/overworld_a_gbc_data.c"
#include "pix/overworld_b_gbc_data.c"

#include "pix/inside_wood_house_map.c"
#include "pix/overworld_a_gbc_map.c"
#include "pix/overworld_b_gbc_map.c"

#include "pix/inside_wood_house_pal.c"
#include "pix/overworld_a_gbc_pal.c"
#include "pix/overworld_b_gbc_pal.c"

const unsigned char *current_map;

#ifdef COMPRESS
// always the same size
UINT8 decompressed_background[80];

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
        if(counter > 0){
            decompressed_background[i] = byte;
            --counter;
        }else{
            if(bytepart == 0){
                hi = (compressed_map[c] >> 4) & 0xF;
                lo = compressed_map[c] & 0xF;
            }else{
                hi = compressed_map[c] & 0xF;
                lo = (compressed_map[c+1] >> 4) & 0xF;
            }
            if((hi & 0x8) == 0){
                // short notation
                if(hi <= 5){
                    decompressed_background[i] = hi+2;
                }else{
                    decompressed_background[i++] = hi*2;
                    decompressed_background[i] = (hi*2) + 1;
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
                // 4x grass
                if(hi == 0xC && lo == 0){
                    decompressed_background[i] = 2;
                    counter = 3;
                    byte = 2;
                }else
                    decompressed_background[i] = 2;
                ++c;
            }
        }

    }
    return decompressed_background;
}
#endif

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
    UINT8 i;
    // tmx
    UINT16 tile;
    // loaded spritesheet
    UINT8 palette;
    unsigned char tiles[4];

    DISPLAY_OFF;
    // load spritesheet
    if (sg->level_y == 4) {
        current_map = overworld_b_gbc_map;
        set_bkg_data(SHEET_START, sizeof(overworld_b_gbc_data) / 16,
                     overworld_b_gbc_data);
        set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        BGP_REG = 0xE4; // 11100100
    } else if (sg->level_y > 4) {
        current_map = inside_wood_house_map;
        set_bkg_data(SHEET_START, sizeof(inside_wood_house_data) / 16,
                     inside_wood_house_data);
        set_bkg_palette(0, 6, inside_wood_house_pal[0]);
        BGP_REG = 0xE4; // 11100100
    } else {
        current_map = overworld_a_gbc_map;
        set_bkg_data(SHEET_START, sizeof(overworld_a_gbc_data) / 16,
                     overworld_a_gbc_data);
        set_bkg_palette(0, 6, overworld_a_gbc_pal[0]);
        BGP_REG = 0xE1;
    }

    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            // load background
            tile = background[(y * WIDTH) + x] - 2;
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
            set_bkg_tiles(x * 2 + 1, y * 2 + 1, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            tiles[0] = SHEET_START + current_map[index];
            tiles[1] = SHEET_START + current_map[index + 2];
            tiles[2] = SHEET_START + current_map[index + 1];
            tiles[3] = SHEET_START + current_map[index + 3];
            set_bkg_tiles(x * 2 + 1, y * 2 + 1, 2, 2, tiles);
        }
    }

    // map scripting
    if (!(sg->collectable & 0x1) && sg->level_x == 1 && sg->level_y == 0) {
        incject_map(2, 2, 29);
    }
    DISPLAY_ON;
}