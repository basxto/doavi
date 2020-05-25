// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "main.h"

#include "hud.h"
#include "logic.h"
#include "map.h"

#include "dev/png2gb/csrc/decompress.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"

#include "pix/characters_data.c"
#include "pix/modular_characters_data.c"
#include "pix/overworld_anim_gbc_data.c"
#include "pix/win_gbc_data.c"

#include "pix/characters_map.c"
#include "pix/modular_characters_map.c"
#include "pix/overworld_anim_gbc_map.c"

#include "pix/characters_pal.c"

#include "pix/hud_pal.c"

#include "strings.h"


#include "level.c"

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];


UINT8 counter;
UINT8 anim_counter;

const Level *current_level;
// since might need to decompress it
const UINT8 *current_background;
extern const unsigned char *current_map;
Savegame *sg;
Savegame sgemu;

void menu() {
    move_win(7, 0);
    HIDE_SPRITES;
    UINT8 ret = smart_write(0, 0, 20, 18, strlen(text_menucont), text_menucont);
    // write_num(12, 1, 3, ret);
    switch (ret) {
    case 2:
        smart_write(0, 0, 20, 18,strlen(text_creditsc), text_creditsc);
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
    //SPRITES_8x16;
    anim_counter = 0;

    BGP_REG = 0xE1; // 11100001
    OBP0_REG = 0xE1;

    //set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
    set_sprite_palette(0, 6, characters_pal[0]);
    set_bkg_palette(7, 1, hud_pal[0]);

    // load tilesets
    set_win_data_rle(WIN_START, win_gbc_data_length, win_gbc_data, 0);
    //set_sprite_data(CHARACTERS_START, sizeof(characters_data) / 16,
     //               characters_data);
    // Test for modular characters
    set_sprite_data(CHARACTERS_START, sizeof(modular_characters_data) / 16,
                    modular_characters_data);

    VBK_REG = 1;
    for (int x = 0; x < 22; ++x) {
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 18, 1, 1, tiles);
        set_bkg_tiles(x, 0, 1, 1, tiles);
    }
    for (int y = 1; y <= 16; ++y) {
        set_bkg_tiles(21, y, 1, 1, tiles);
        set_bkg_tiles(0, y, 1, 1, tiles);
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + 6;
    for (int x = 0; x < 22; ++x) {
        set_bkg_tiles(x, 17, 1, 1, tiles);
        set_bkg_tiles(x, 18, 1, 1, tiles);
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
    current_background = decompress(current_level->background);
    for(UINT8 i = 1; i < 5; ++i){
        // disable characters
        sg->character[i].sprite = 0xFF;
        sg->character[i].offset_x = 0;
        sg->character[i].offset_y = 0;
        sg->character[i].direction = 0;
        render_character(i);
    }
    load_map(current_background);
}

// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const UINT8 index) {
    Character *chrctr = &sg->character[index];
    if(chrctr->sprite == 0xFF){
        // move all four sprites outside of the view
        for(UINT8 i = 0; i < 4; ++i)
            move_sprite(chrctr->sprite_index+i, 0, 0);
    }else{
        UINT8 index = chrctr->sprite_index-1;
        UINT8 x = $(8) + (chrctr->x) * $(16) + chrctr->offset_x;
        // shifted to top by 2px to look like standing on the tile
        UINT8 y = ((chrctr->y) + $(1)) * $(16) + chrctr->offset_y - $(2);
        UINT8 mapping;
        UINT8 base = (10*8) + (chrctr->sprite*19) + ((chrctr->direction&0x03)*4);
        UINT8 palette = chrctr->palette&0x0F;
        for(UINT8 i = 0; i < 2; ++i){
            mapping = modular_characters_map[base];
            set_sprite_tile(++index, CHARACTERS_START+(mapping&0x7F));
            set_sprite_prop(index, palette | (mapping&0x80?0x20:0x0));
            move_sprite(index, x, y);
            x+=8;
            base+=2;
        }
        x-=16;
        y+=8;
        base = (8*2) + (2*chrctr->direction);
        palette = chrctr->palette>>4;
        for(UINT8 j = 0; j < 2; ++j){
            mapping = modular_characters_map[base++];
            set_sprite_tile(++index, CHARACTERS_START+(mapping&0x7F));
            set_sprite_prop(index, palette | (mapping&0x80?0x20:0x0));
            move_sprite(index, x, y);
            x+=8;
        }
    }
}

UINT8 is_free(const UINT8 x, const UINT8 y) {
    if (y >= HEIGHT || x >= WIDTH) {
        return 0;
    }
    UINT8 index = (y) * WIDTH + (x);
    UINT8 tile = current_background[index];
    //write_num(12, 1, 3, tile);
    if ((current_level->collision[index / $(8)] & (1 << (index % $(8)))) == 0 && tile != 16 && tile != 27) {
        // check entity collision
        for(UINT8 i = 1; i < 5; ++i)
            if(sg->character[i].sprite != 0xFF && sg->character[i].x == x && sg->character[i].y == y)
                return 0;
        return 1;
    }
    return 0;
}

UINT8 move_character(const UINT8 index, const INT8 x, const INT8 y) {
    Character *chrctr = &sg->character[index];
    if (chrctr->x == 0 && x < 0) {
        sg->level_x--;
        chrctr->x += WIDTH + x;
        wait_vbl_done();
        render_character(index);
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        sg->level_x++;
        chrctr->x += -WIDTH + x;
        wait_vbl_done();
        render_character(index);
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        sg->level_y--;
        chrctr->y += HEIGHT + y;
        wait_vbl_done();
        render_character(index);
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        sg->level_y++;
        chrctr->y += -HEIGHT + y;
        wait_vbl_done();
        render_character(index);
        change_level();
        return 0;
    }
    //write_num(12, 1, 3, tile);
    if (is_free((chrctr->x + x),(chrctr->y + y)) != 0) {
        for (int i = 4; i != 0; --i) {
            chrctr->offset_x += x * $(4);
            chrctr->offset_y += y * $(4);
            // a little jump in the walk
            wait_vbl_done();
            render_character(index);
        }
        chrctr->offset_x = 0;
        chrctr->offset_y = 0;
        chrctr->x += x;
        chrctr->y += y;
        wait_vbl_done();
        render_character(index);
        return 0;
    } else {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
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
        replace_tile(2, 1, anim_counter);
        replace_tile(1, 0, anim_counter);
        replace_tile(SHEET_WIDTH * 3 + 4, 2, anim_counter);
    }else if (current_map == overworld_b_gbc_map) {
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
    ++anim_counter;
    anim_counter %= ANIM_WIDTH;
}

void timer_isr() {
    tick_music();
}

void vblank_isr(){
    if (counter++ == 0) {
        tick_animate();
    }
    counter %= $(32);
}

void main() {
    //sg = (Savegame *)0xa000;
    sg = &sgemu;
    // load savegame
    //ENABLE_RAM_MBC1;
    if (sg->magic != 'V') {
        sg->level_x = 1;
        sg->level_y = 4;

        sg->character[0].x = 4;
        sg->character[0].y = 4;
        sg->character[0].sprite = 0;
        sg->character[0].direction = 0;
        sg->character[0].palette = (2<<4)|2;
        sg->character[0].sprite_index = 36;
        sg->character[0].offset_x = 0;
        sg->character[0].offset_y = 0;

        for(UINT8 i = 0; i < 4; ++i){
            sg->character[i+1].sprite_index = (i*4);
        }

        sg->lives = 5;
        sg->tpaper = 0;

        sg->collectable = 0;

        sg->magic = 'V';
    }
    counter = 0;
    init_screen();
    init_hud();

    //init_music(&the_journey_begins);
    //init_music(&cosmicgem_voadi);

    render_character(0);
    change_level();

    SHOW_BKG;
    SHOW_WIN;
    DISPLAY_ON;

    // configure interrupt
    // triggers interrupt when it reaches 0xFF
    TMA_REG = TIMA_REG = 0xE3;
    TAC_REG = 0x4 | 0x0; // 4096 Hz
    // enable timer interrupt
    disable_interrupts();
    add_TIM(timer_isr);
    add_VBL(vblank_isr);
    //add_
    enable_interrupts();
    // gbdk needs VBL iflag
    set_interrupts(VBL_IFLAG | TIM_IFLAG);
    // UINT8 rom = 1;

    move_win(7, 0);
    space_area(0, 0, 20, 18);
    // measure time
    //write_hex(0,0,2,sys_time>>8);
    //write_hex(2,0,2,sys_time&0xFF);

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
            sg->character[0].direction = 3;
            if (move_player(1, 0) == 1)
                render_character(0);
            delay(100);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            sg->character[0].direction = 1;
            if (move_player(-1, 0) == 1)
                render_character(0);
            delay(100);
            break;
        case J_UP: // If joypad() is equal to UP
            sg->character[0].direction = 2;
            if (move_player(0, -1) == 1)
                render_character(0);
            delay(100);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            sg->character[0].direction = 0;
            if (move_player(0, 1) == 1)
                render_character(0);
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
        wait_vbl_done();
    }
}
