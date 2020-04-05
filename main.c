// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "hud.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/music/the_journey_begins.c"

#include "pix/characters_data.c"
#include "pix/overworld_a_gbc_data.c"
#include "pix/overworld_anim_gbc_data.c"
#include "pix/overworld_b_gbc_data.c"
#include "pix/inside_wood_house_data.c"
#include "pix/win_gbc_data.c"

#include "pix/characters_map.c"
#include "pix/overworld_a_gbc_map.c"
#include "pix/overworld_anim_gbc_map.c"
#include "pix/overworld_b_gbc_map.c"
#include "pix/inside_wood_house_map.c"

#include "pix/characters_pal.c"
#include "pix/overworld_a_gbc_pal.c"
#include "pix/overworld_b_gbc_pal.c"
#include "pix/inside_wood_house_pal.c"

#include "strings.c"

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

#include "level.c"

void load_map(const UINT8 background[]);

UINT8 used_sprites;
UINT8 counter;
UINT8 anim_counter;

Level *current_level;
const unsigned char *current_map;

typedef struct {
    UINT8 x; // position
    UINT8 y;
    UINT8 direction;
    UINT8 palette;
    UINT8 sprite; // sprite character section
    // maybe completely remove this and do this in animation tick
    UINT8 sprite_index;
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
    // allow player to have 8 items
    UINT8 items[8];
    UINT8 selected_item;
} Savegame;
// Savegame noram;
Savegame *sg;

void menu(){
    move_win(7, 0);
    HIDE_SPRITES;
    UINT8 ret = smart_write(0, 0, 20, 18, strlen(text_menucont), text_menucont);
    //write_num(12, 1, 3, ret);
    switch(ret){
        case 2:
            smart_write(0, 0, 20, 18, strlen(text_creditsc), text_creditsc);
            waitpad(J_A);
            delay(100);
            break;
    }
    draw_hud(sg->lives, sg->tpaper);
    SHOW_SPRITES;
}

void screen_shake(){
    for(int i = 0; i < 8; ++i){
        scroll_bkg(-2,0);
        wait_vbl_done();
        wait_vbl_done();
        scroll_bkg(0,-2);
        wait_vbl_done();
        scroll_bkg(+4,0);
        wait_vbl_done();
        wait_vbl_done();
        scroll_bkg(0,+4);
        wait_vbl_done();
        scroll_bkg(-2,0);
        wait_vbl_done();
        scroll_bkg(0,-2);
    }
}

void init_screen(){
    UINT8 tiles[0] ={1};
    HIDE_BKG;
    HIDE_WIN;
    HIDE_SPRITES;
    DISPLAY_OFF;
    cgb_compatibility();
    SPRITES_8x16;
    used_sprites = 0;
    anim_counter = 0;

    BGP_REG = 0xE1; // 11100001
    OBP0_REG = 0xE1;

    set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
    set_sprite_palette(0, 6, characters_pal[0]);

    // load tilesets
    set_win_data(WIN_START, sizeof(win_gbc_data) / 16, win_gbc_data);
    set_sprite_data(CHARACTERS_START, sizeof(characters_data) / 16,
                    characters_data);

    VBK_REG = 1;
    for (int x = 0; x < 22; ++x){
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 0, 1, 1, tiles);
    }
    for (int y = 1; y <= 16; ++y){
        set_bkg_tiles(21, y, 1, 1, tiles);
        set_bkg_tiles(0, y, 1, 1, tiles);
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + (' '-8);
    for (int x = 0; x < 22; ++x){
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 0, 1, 1, tiles);
    }
    for (int y = 1; y <= 16; ++y){
        set_bkg_tiles(21, y, 1, 1, tiles);
        set_bkg_tiles(0, y, 1, 1, tiles);
    }
    move_bkg(8,8);
}

void change_level() {
    current_level = &level[sg->level_y][sg->level_x];
    load_map(current_level->background);
}

// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const Character *chrctr) {
    UINT8 base = chrctr->sprite * 4 * 4 + chrctr->direction * 4;
    set_sprite_tile(chrctr->sprite_index,
                    CHARACTERS_START + characters_map[base]);
    move_sprite(chrctr->sprite_index, 8 + (chrctr->x) * 16,
                16 + (chrctr->y) * 16);
    set_sprite_prop(chrctr->sprite_index, chrctr->palette);
    set_sprite_tile(chrctr->sprite_index + 1,
                    CHARACTERS_START + characters_map[base + 2]);
    move_sprite(chrctr->sprite_index + 1, 8 + (chrctr->x) * 16 + 8,
                16 + (chrctr->y) * 16);
    set_sprite_prop(chrctr->sprite_index + 1, chrctr->palette);
}

UINT8 move_character(Character *chrctr, const INT8 x, const INT8 y,
                     const UINT8 *collision) {
    if (chrctr->x == 0 && x < 0) {
        sg->level_x--;
        chrctr->x += WIDTH + x;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        sg->level_x++;
        chrctr->x += -WIDTH + x;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        sg->level_y--;
        chrctr->y += HEIGHT + y;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        sg->level_y++;
        chrctr->y += -HEIGHT + y;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    UINT8 index = (chrctr->y + y) * WIDTH + (chrctr->x + x);
    if ((collision[index / 8] & (1 << (index % 8))) == 0) {
        chrctr->x += x;
        chrctr->y += y;
        render_character(&(sg->player));
        return 0;
    } else {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }
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

    // reset all sprites used_sprites
    for (i = 0; i < used_sprites; ++i)
        move_sprite(used_sprites, 0, 0);

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
            if (current_map == overworld_a_gbc_map && palette > 3){
                palette = 2;
            }
            // inside house
            if (current_map == inside_wood_house_map){
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

void interact() {
    UINT8 x = sg->player.x;
    UINT8 y = sg->player.y;
    UINT8 tile;
    switch (sg->player.direction) {
    case 0:
        y++;
        break;
    case 1:
        y--;
        break;
    case 2:
        x--;
        break;
    case 3:
        x++;
        break;
    }
    tile = current_level->background[(y * WIDTH) + x];
    //write_num(8, 1, 3, tile);
    if (tile == 18) {
        if (sg->level_x == 0 && sg->level_y == 0) {
            dialog(strlen(text_whatsupn), text_whatsupn, strlen(text_sign),
                   text_sign, 1);
        } else {
            dialog(strlen(text_hellowor), text_hellowor, strlen(text_sign),
                   text_sign, 1);
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 26) {
        screen_shake();
        dialog(strlen(text_somebody), text_somebody, strlen(text_grave),
               text_grave, 2);
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 30) {
        dialog(strlen(text_burnever), text_burnever, strlen(text_flame),
               text_flame, 3);
        draw_hud(sg->lives, sg->tpaper);
        //reset();
    }
    if (tile == 32) {
        if (!(sg->collectable & 0x1) && sg->level_x == 1 && sg->level_y == 0) {
            incject_map(2, 2, 30);
            sg->collectable |= 0x1;
            sg->tpaper++;
            draw_hud(sg->lives, sg->tpaper);
            blinger(0x05 | note_a, 4, 0x05 | note_b, 5, 0x04 | note_e);
        }
    }
}

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

inline void tick_animate() {
    if (current_map == overworld_a_gbc_map) {
        replace_tile(1, 0, anim_counter);
        replace_tile(2, 1, anim_counter);
        replace_tile(SHEET_WIDTH * 3 + 4, 2, anim_counter);
    }
    if (current_map == overworld_b_gbc_map) {
        replace_tile(SHEET_WIDTH * 3 + 7, 3, anim_counter);
        replace_tile(SHEET_WIDTH * 3 + 3, 4, anim_counter);
        // shore waves
        replace_subtile(SHEET_WIDTH * 3 + 4, 5, anim_counter, 0);
        replace_subtile(SHEET_WIDTH * 3 + 4, 5, anim_counter, 2);
        replace_subtile(SHEET_WIDTH * 3 + 5, 6, anim_counter, 0);
        replace_subtile(SHEET_WIDTH * 3 + 5, 6, anim_counter, 1);
        replace_subtile(SHEET_WIDTH * 3 + 6, 7, anim_counter, 2);
        replace_subtile(SHEET_WIDTH * 3 + 6, 7, anim_counter, 3);
    }

    anim_counter = (anim_counter + 1) % ANIM_WIDTH;
}

void timer_isr() {
    tick_music();
    if (counter % 8 == 0) {
        tick_animate();
    }
    counter++;
    counter %= 20;
}

void main() {
    sg = (Savegame *)0xa000;
    // load savegame
    ENABLE_RAM_MBC1;
    if (sg->magic != 'V') {
        sg->level_x = 1;
        sg->level_y = 4;

        sg->player.x = 4;
        sg->player.y = 4;
        sg->player.sprite = 1;
        sg->player.direction = 0;
        sg->player.palette = 2;
        sg->player.sprite_index = 38;

        sg->lives = 5;
        sg->tpaper = 0;

        sg->collectable = 0;

        sg->magic = 'V';
    }
    current_level = &level[sg->level_y][sg->level_x];
    counter = 0;
    init_screen();
    init_hud();

    init_music(&the_journey_begins);

    render_character(&(sg->player));
    load_map(current_level->background);


    SHOW_BKG;
    SHOW_WIN;
    DISPLAY_ON;

    // configure interrupt
    TIMA_REG = TMA_REG = 0x1A;
    TAC_REG = 0x4 | 0x0; // 4096 Hz
    // enable timer interrupt
    disable_interrupts();
    add_TIM(timer_isr);
    enable_interrupts();
    // gbdk needs VBL iflag
    set_interrupts(VBL_IFLAG | TIM_IFLAG);
    //UINT8 rom = 1;

    move_win(7, 0);
    space_area(0, 0, 20, 18);
    smart_write(3, 4, 20, 2, strlen(text_desserto), text_desserto);

    smart_write(5, 12, 20, 2, strlen(text_bybasxto), text_bybasxto);
    waitpad(J_A);
    delay(100);
    smart_write(0, 0, 20, 18, strlen(text_youaream), text_youaream);
    waitpad(J_A);
    delay(100);
    init_hud();
    draw_hud(sg->lives, sg->tpaper);

    SHOW_SPRITES;

    while (1) {

        switch (joypad()) {
        case J_RIGHT: // If joypad() is equal to RIGHT
            sg->player.direction = 3;
            if (move_character(&(sg->player), 1, 0, current_level->collision) ==
                1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            sg->player.direction = 2;
            if (move_character(&(sg->player), -1, 0,
                               current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_UP: // If joypad() is equal to UP
            sg->player.direction = 1;
            if (move_character(&(sg->player), 0, -1,
                               current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            sg->player.direction = 0;
            if (move_character(&(sg->player), 0, 1, current_level->collision) ==
                1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_A: // If joypad() is equal to DOWN
            interact();
            delay(100);
            break;
        // for bank testing
        case J_START:
            menu();
            delay(100);
        default:
            break;
        }
    }
}
