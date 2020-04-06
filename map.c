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