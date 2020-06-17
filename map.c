#include <string.h>

#include "map.h"
#include "main.h"

#include "pix/pix.h"

#include "music/songs.h"

#include "dev/png2gb/csrc/decompress.h"
#include "unpb16.h"

const unsigned char *current_map;
const unsigned char *loaded_map;
// always the same size
UINT8 decompressed_background[80];
//TODO: calculation of tileset length is wrong
UINT8 decompressed_tileset[166*16];

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
    set_bkg_tiles(x * $(2) + 1, y * $(2) + 1, 2, 2, tiles);
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
    set_bkg_tiles(x * $(2) + 1, y * $(2) + 1, 2, 2, tiles);
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
    BGP_REG = 0xE4; // 11100100
    // load spritesheet
    if (sg->level_y == 4) {//0b0100
        if(current_map != overworld_b_gbc_map){
            init_music(&cosmicgem_voadi);
            current_map = overworld_b_gbc_map;
            loaded_map = overworld_b_gbc_map;
            pb16_unpack_block(overworld_b_gbc_pb16_data_length, overworld_b_gbc_pb16_data, decompressed_tileset);
            set_bkg_data(SHEET_START, overworld_b_gbc_pb16_data_length,
                        decompressed_tileset);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else if (sg->level_y == 5) {//0b0101
        if(current_map != inside_wood_house_map){
            current_map = inside_wood_house_map;
            loaded_map = inside_wood_house_map;
            pb16_unpack_block(inside_wood_house_pb16_data_length, inside_wood_house_pb16_data, decompressed_tileset);
            set_bkg_data(SHEET_START, inside_wood_house_pb16_data_length,
                        decompressed_tileset);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else if (sg->level_y > 5) {
        if(current_map != overworld_cave_map){
            current_map = overworld_cave_map;
            loaded_map = overworld_cave_map;
            pb16_unpack_block(overworld_cave_pb16_data_length, overworld_cave_pb16_data, decompressed_tileset);
            set_bkg_data(SHEET_START, overworld_cave_pb16_data_length,
                        decompressed_tileset);
            set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        }
    } else {
        if(current_map != overworld_a_gbc_map){
            if(current_map == overworld_b_gbc_map)
                init_music(&the_journey_begins);
            current_map = overworld_a_gbc_map;
            loaded_map = overworld_a_gbc_map;
            pb16_unpack_block(overworld_a_gbc_pb16_data_length, overworld_a_gbc_pb16_data, decompressed_tileset);
            set_bkg_data(SHEET_START, overworld_a_gbc_pb16_data_length,
                        decompressed_tileset);
            set_bkg_palette(0, 6, overworld_a_gbc_pal[0]);
        }
        BGP_REG = 0xE1;
    }

    for (y = 0; y < HEIGHT; ++y) {
        UINT8 tmp1 = y * WIDTH;
        UINT8 tmp2 = y * 2 + 1;
        for (x = 0; x < WIDTH; ++x) {
            UINT8 tmpx = x * $(2) + $(1);
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
            if(palette > 3)
                if (current_map == overworld_a_gbc_map) {
                    // houses extension
                    palette = 2;
                }
                else if (current_map == overworld_b_gbc_map) {
                    // bottle
                    palette = 1;
                }
            // inside house
            if (current_map == inside_wood_house_map) {
                palette = 2;
                if(tile == 7 || tile >= 32)
                    palette = 4;//carpet and flame
                if(tile == 14)
                    palette = 1;//plant
            }
            if (current_map == overworld_cave_map) {
                palette = 2;
            }
            tiles[0] = tiles[1] = tiles[2] = tiles[3] = palette;
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            UINT8 j = 0;// goes 0 2 1 3
            for(UINT8 i = 0; i < 4; ++i){
                //tiles[0] = SHEET_START + current_map[index + j];
                tiles[i] = SHEET_START + current_map[index + j];
                if(j==2)
                    --j;
                else
                    j+=2;
            }
            set_bkg_tiles(tmpx, tmp2, 2, 2, tiles);
        }
    }

    // map scripting
    if (!(sg->chest & 0x1) && sg->level_x == 1 && sg->level_y == 0) {
        incject_map(2, 2, 29);
    }
    if (!(sg->chest & 1<<1) && sg->level_x == 1 && sg->level_y == 0) {
        incject_map(1, 6, 29);
    }
    if (!(sg->chest & 1<<2) && sg->level_x == 2 && sg->level_y == 0) {
        incject_map(3, 4, 17);
    }
    DISPLAY_ON;

    if(((sg->progress[0] & PRGRS_GHOST)==0x10) && sg->level_x == 0 && sg->level_y == 0){
        // spawn ghost
        sg->character[1].x = 4;
        sg->character[1].y = 2;
        sg->character[1].sprite = 2;
        sg->character[1].direction = 7<<2;//ghost bottom
        sg->character[1].palette = 3<<4 | 3;

        render_character(1);
    }
    else if(sg->level_x == 4 && sg->level_y == 0){
        // spawn timetravel robot
        sg->character[1].x = 4;
        sg->character[1].y = 2;
        sg->character[1].sprite = 4;
        sg->character[1].palette = 1<<4 | 1;
        // spawn broken timetravel robot
        sg->character[2].x = 8;
        sg->character[2].y = 6;
        sg->character[2].sprite = 3;
        sg->character[2].palette = 1<<4 | 1;

        render_character(1);
        render_character(2);
    }
    else if(sg->level_x == 4 && sg->level_y == 5){
        // spawn rachel
        sg->character[1].x = 4;
        sg->character[1].y = 3;
        sg->character[1].sprite = 7;
        sg->character[1].palette = 1<<4 | 1;

        render_character(1);
    }
}