#ifndef MAIN_H
#define MAIN_H

#include <gb/gb.h>

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HEIGHT (8)
#define WIDTH (10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH (16)
#define TRANSPARENT (RGB(12, 25, 0))

#define CHARACTERS_START (0)
#define SHEET_START (128)
// width in 16x16 blocks
#define SHEET_WIDTH (8)
#define ANIM_WIDTH (4)

typedef struct {
    UINT8 x; // position
    UINT8 y;
    UINT8 direction;
    UINT8 palette;
    // 0xFF disables this character
    UINT8 sprite; // sprite character section
    // < 39 (always uses 2 sprites)
    UINT8 sprite_index;
    INT8 offset_x;
    INT8 offset_y;
} Character;

typedef struct {
    char magic;
    UINT8 level_x;
    UINT8 level_y;
    UINT8 lives;
    UINT8 tpaper;
    // each bit for one
    UINT8 collectable;
    Character player;
    Character character[4];
    // allow player to have 8 items
    UINT8 items[8];
    UINT8 selected_item;
} Savegame;

extern Savegame *sg;

void menu();
void screen_shake();
void init_screen();
void change_level();
// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const Character *chrctr);
UINT8 is_free( const UINT8 x, const UINT8 y);
UINT8 move_character(Character *chrctr, const INT8 x, const INT8 y,
                     const UINT8 *collision);

// index of tile in spritesheet; index of tile in animation sheet
// 16x16 block indices
#define replace_tile(index, indexa, counter)                                   \
    (set_bkg_data(                                                             \
        SHEET_START + current_map[(index)*4], 4,                               \
        &overworld_anim_gbc_data                                               \
            [overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (counter)) * 4] *   \
             16]))

// for compressed tiles
#define replace_subtile(index, indexa, counter, offset)                        \
    (set_bkg_data(                                                             \
        SHEET_START + current_map[(index)*4 + offset], 1,                      \
        &overworld_anim_gbc_data                                               \
            [overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (counter)) * 4 +    \
                                    offset] *                                  \
             16]))

inline void tick_animate();
void timer_isr();
void main();
#endif