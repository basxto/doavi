#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HEIGHT U8(8)
#define WIDTH U8(10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH U8(16)
#define TRANSPARENT (RGB(12, 25, 0))

#define CHARACTERS_START U8(0)
#define ITEMS_START U8(128-4)
#define SHEET_START U8(128)
// width in 16x16 blocks
#define SHEET_WIDTH U8(8)
#define ANIM_WIDTH U8(4)

typedef struct {
    uint8_t x; // position
    uint8_t y;
    // 0 down 1 left 2 up 3 right
    // 0xFC selects body sprites
    uint8_t direction;
    // upper nibble for head and lower for body
    uint8_t palette;
    // 0xFF disables this character
    // selects head sprite
    uint8_t sprite;
    // < 39 (always uses 2 sprites)
    uint8_t sprite_index;
    int8_t offset_x;
    int8_t offset_y;
} Character;


// progress[0]
#define PRGRS_BTL (1<<0) // bottle message read
#define PRGRS_T1 (1<<1) // cleared access to T-1
#define PRGRS_T0 (1<<2) // destroyed T0
#define PRGRS_LVR (1<<3) // lever activated

// ghost status
#define PRGRS_GHOST (0x3<<4)
#define IS_PRGRS_GHOST(x)  (U8((sg->progress[0]) & (0x3<<4)) == (x)<<4)
#define SET_PRGRS_GHOST(x) (sg->progress[0] = (sg->progress[0] & 0xCF) | (x)<<4)
// time status
#define PRGRS_TIME (0x3<<6)
#define IS_PRGRS_TIME(x)   (U8((sg->progress[0]) & (0x3<<6)) == (x)<<6)
#define SET_PRGRS_TIME(x)  (sg->progress[0] = (sg->progress[0] & 0x3F) | (x)<<6)

#define ITEM_NOTHING    (0)
#define ITEM_SWORD      (1)
#define ITEM_POWER      (2)
#define ITEM_FLINT      (3)
#define ITEM_KEY        (4)
#define ITEM_SHOVEL     (5)

// 68 bytes
typedef struct {
    char name[10];
    uint8_t level_x;
    uint8_t level_y;
    uint8_t lives;
    uint8_t tpaper;
    // character[0]:
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    // allow player to have 8 items
    uint8_t item[8];
    uint8_t selected_item;
    uint8_t chest; // 7
    uint8_t flame; // 8
    // from right to left, top to bottom
    //4 barrels
    //ghost (2bit) 0: not there; 1: appearance; 2: disappearance
    //time zone (2bits) 0: normal; 1: past; 2: desert island

    //rest through PRGGS_ defines

    uint8_t progress[2];
    bool cheat;
} Savegame;

typedef struct {
    char magic;
    // up to 16 slots
    // 0: unused
    // 1: used
    uint16_t slots;
} Saveslots;

extern Savegame *sg;

// it's too much overhead to access the savegame directly
extern uint8_t level_x;
extern uint8_t level_y;
extern uint8_t lives;
extern uint8_t tpaper;
//0 is player; rest are NPCs
extern Character character[5];
extern uint8_t item[8];
extern uint8_t selected_item;
extern uint8_t chest;
extern uint8_t flame;
extern uint8_t progress[2];

extern bool cheat;


extern uint8_t current_chest;
extern uint8_t current_flame;

void menu();
void screen_wobble();
void screen_shake();
void init_screen();
void change_level();
uint8_t get_selected_item();
// we not allow so many characters, maybe just give id?
// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const uint8_t index);
uint8_t is_free( const uint8_t x, const uint8_t y);
uint8_t move_character(const uint8_t index, const int8_t x, const int8_t y);
void save_sg();
void load_sg();

inline void tick_animate();
void timer_isr();
void main();
#endif