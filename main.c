// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "main.h"

#include "hud.h"
#include "logic.h"
#include "map.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/music/the_journey_begins.c"

#include "pix/characters_data.c"
#include "pix/overworld_anim_gbc_data.c"
#include "pix/win_gbc_data.c"

#include "pix/characters_map.c"
#include "pix/overworld_anim_gbc_map.c"

#include "pix/characters_pal.c"

#include "strings.h"


#include "level.c"

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];


UINT8 counter;
UINT8 anim_counter;

Level *current_level;
extern const unsigned char *current_map;
Savegame *sg;

void menu() {
    move_win(7, 0);
    HIDE_SPRITES;
    UINT8 ret = smart_write(0, 0, 20, 18, strlen(text_menucont), text_menucont);
    // write_num(12, 1, 3, ret);
    switch (ret) {
    case 2:
        smart_write(0, 0, 20, 18, strlen(text_creditsc), text_creditsc);
        waitpad(J_A);
        delay(100);
        break;
    }
    draw_hud(sg->lives, sg->tpaper);
    SHOW_SPRITES;
}

void screen_shake() {
    for (int i = 0; i < 8; ++i) {
        scroll_bkg(-2, 0);
        wait_vbl_done();
        wait_vbl_done();
        scroll_bkg(0, -2);
        wait_vbl_done();
        scroll_bkg(+4, 0);
        wait_vbl_done();
        wait_vbl_done();
        scroll_bkg(0, +4);
        wait_vbl_done();
        scroll_bkg(-2, 0);
        wait_vbl_done();
        scroll_bkg(0, -2);
    }
}

void init_screen() {
    UINT8 tiles[0] = {1};
    HIDE_BKG;
    HIDE_WIN;
    HIDE_SPRITES;
    DISPLAY_OFF;
    cgb_compatibility();
    SPRITES_8x16;
    anim_counter = 0;

    BGP_REG = 0xE1; // 11100001
    OBP0_REG = 0xE1;

    //set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
    set_sprite_palette(0, 6, characters_pal[0]);

    // load tilesets
    set_win_data(WIN_START, sizeof(win_gbc_data) / 16, win_gbc_data);
    set_sprite_data(CHARACTERS_START, sizeof(characters_data) / 16,
                    characters_data);

    VBK_REG = 1;
    for (int x = 0; x < 22; ++x) {
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 0, 1, 1, tiles);
    }
    for (int y = 1; y <= 16; ++y) {
        set_bkg_tiles(21, y, 1, 1, tiles);
        set_bkg_tiles(0, y, 1, 1, tiles);
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + (' ' - 8);
    for (int x = 0; x < 22; ++x) {
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 0, 1, 1, tiles);
    }
    for (int y = 1; y <= 16; ++y) {
        set_bkg_tiles(21, y, 1, 1, tiles);
        set_bkg_tiles(0, y, 1, 1, tiles);
    }
    move_bkg(8, 8);
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
    move_sprite(chrctr->sprite_index, 8 + (chrctr->x) * 16 + chrctr->offset_x,
                16 + (chrctr->y) * 16 + chrctr->offset_y);
    set_sprite_prop(chrctr->sprite_index, chrctr->palette);
    set_sprite_tile(chrctr->sprite_index + 1,
                    CHARACTERS_START + characters_map[base + 2]);
    move_sprite(chrctr->sprite_index + 1,
                8 + (chrctr->x) * 16 + chrctr->offset_x + 8,
                16 + (chrctr->y) * 16 + chrctr->offset_y);
    set_sprite_prop(chrctr->sprite_index + 1, chrctr->palette);
}

UINT8 move_character(Character *chrctr, const INT8 x, const INT8 y,
                     const UINT8 *collision) {
    if (chrctr->x == 0 && x < 0) {
        sg->level_x--;
        chrctr->x += WIDTH + x;
        wait_vbl_done();
        render_character(chrctr);
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        sg->level_x++;
        chrctr->x += -WIDTH + x;
        wait_vbl_done();
        render_character(chrctr);
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        sg->level_y--;
        chrctr->y += HEIGHT + y;
        wait_vbl_done();
        render_character(chrctr);
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        sg->level_y++;
        chrctr->y += -HEIGHT + y;
        wait_vbl_done();
        render_character(chrctr);
        change_level();
        return 0;
    }
    UINT8 index = (chrctr->y + y) * WIDTH + (chrctr->x + x);
    if ((collision[index / 8] & (1 << (index % 8))) == 0) {
        for (int i = 0; i < 4; ++i) {
            chrctr->offset_x += x * 4;
            chrctr->offset_y += y * 4;
            // a little jump in the walk
            wait_vbl_done();
            render_character(chrctr);
        }
        chrctr->offset_x = 0;
        chrctr->offset_y = 0;
        chrctr->x += x;
        chrctr->y += y;
        wait_vbl_done();
        render_character(chrctr);
        return 0;
    } else {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }
}

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
        sg->player.offset_x = 0;
        sg->player.offset_y = 0;

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
    // UINT8 rom = 1;

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
            if (move_player(1, 0, current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            sg->player.direction = 2;
            if (move_player(-1, 0, current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_UP: // If joypad() is equal to UP
            sg->player.direction = 1;
            if (move_player(0, -1, current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            sg->player.direction = 0;
            if (move_player(0, 1, current_level->collision) == 1)
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
