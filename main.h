#ifndef MAIN_H
#define MAIN_H

#include <gb/gb.h>
#include "utils.h"

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HEIGHT $(8)
#define WIDTH $(10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH $(16)
#define TRANSPARENT (RGB(12, 25, 0))

#define CHARACTERS_START $(0)
#define SHEET_START $(128)
// width in 16x16 blocks
#define SHEET_WIDTH $(8)
#define ANIM_WIDTH $(4)

typedef struct {
    UINT8 x; // position
    UINT8 y;
    // 0 down 1 left 2 up 3 right
    // 0xFC selects body sprites
    UINT8 direction;
    // upper nibble for head and lower for body
    UINT8 palette;
    // 0xFF disables this character
    // selects head sprite
    UINT8 sprite;
    // < 39 (always uses 2 sprites)
    UINT8 sprite_index;
    INT8 offset_x;
    INT8 offset_y;
} Character;


// progress[0]
#define PRGRS_BTL (1<<0) // bottle message read
#define PRGRS_T1 (1<<1) // cleared access to T-1
#define PRGRS_T0 (1<<2) // destroyed T0
#define PRGRS_LVR (1<<3) // lever activated

// ghost status
#define PRGRS_GHOST (0x3<<4)
#define IS_PRGRS_GHOST(x)  (((sg->progress[0]) & (0x3<<4)) == (x)<<4)
#define SET_PRGRS_GHOST(x) (sg->progress[0] = (sg->progress[0] & 0xCF) | (x)<<4)
// time status
#define PRGRS_TIME (0x3<<6)
#define IS_PRGRS_TIME(x)   (((sg->progress[0]) & (0x3<<6)) == (x)<<6)
#define SET_PRGRS_TIME(x)  (sg->progress[0] = (sg->progress[0] & 0x3F) | (x)<<6)

// 68 bytes
typedef struct {
    char magic;
    char name[10];
    UINT8 level_x;
    UINT8 level_y;
    UINT8 lives;
    UINT8 tpaper;
    //0 is player; rest are NPCs
    Character character[5];
    // allow player to have 8 items
    UINT8 items[8];
    UINT8 selected_item;
    UINT8 chest; // 7
    UINT8 flame; // 8
    // from right to left, top to bottom
    //4 barrels
    //ghost (2bit) 0: not there; 1: appearance; 2: disappearance
    //time zone (2bits) 0: normal; 1: past; 2: desert island

    //rest through PRGGS_ defines

    UINT8 progress[2];
} Savegame;

extern Savegame *sg;

void menu();
void screen_shake();
void init_screen();
void change_level();
// we not allow so many characters, maybe just give id?
// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const UINT8 index);
UINT8 is_free( const UINT8 x, const UINT8 y);
UINT8 move_character(const UINT8 index, const INT8 x, const INT8 y);

inline void tick_animate();
void timer_isr();
void main();
#endif