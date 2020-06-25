// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

#include "hud.h"
#include "logic.h"
#include "map.h"

#include "dev/png2gb/csrc/decompress.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"

#include "pix/pix.h"

#include "strings.h"

#include "unpb16.h"

#include "level.h"

extern const Level level[][7];

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];

//TODO: pretty much a hack
extern UINT8 decompressed_tileset[128*16];


UINT8 counter;
UINT8 anim_counter;

// since might need to decompress it
const UINT8 *current_background;
// since we want to modify it
UINT8 current_collision[10];
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
        waitpad_any(J_A);
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
    pb16_unpack_sprite_data(CHARACTERS_START, modular_characters_data_length, decompressed_tileset, modular_characters_data);

    VBK_REG = 1;
    // make it all black
    for (int x = 0; x < 0x20; ++x) {
        for (int y = 0; y <= 0x1F; ++y) {
            set_bkg_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + 6;
    for (int x = 0; x < 0x20; ++x) {
        for (int y = 0; y <= 0x1F; ++y) {
            set_bkg_tiles(x, y, 1, 1, tiles);
        }
    }
    move_bkg(8, 0);
}

void change_level() {
    memcpy(current_collision, level[sg->level_y][sg->level_x].collision, 10);
    current_background = decompress(level[sg->level_y][sg->level_x].background);
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
    //write_num(12, 1, 3, tile);
    if ((current_collision[index / $(8)] & (1 << (index % $(8)))) == 0) {
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
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        sg->level_x++;
        chrctr->x += -WIDTH + x;
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        sg->level_y--;
        chrctr->y += HEIGHT + y;
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        sg->level_y++;
        chrctr->y += -HEIGHT + y;
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
void replace_tile(UINT8 index, UINT8 indexa){
    UINT8 first = SHEET_START + current_map[(index)*4];
    UINT8 amount = 4;
    const unsigned char *data = &overworld_anim_gbc_data[overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (anim_counter)) * 4] * 16];
    set_bkg_data(first, amount,data);
}

// for deduplicated tiles
void replace_subtile(UINT8 index, UINT8 indexa, UINT8 offset){
    UINT8 first = SHEET_START + current_map[(index)*4 + offset];
    UINT8 amount = 1;
    const unsigned char *data = &overworld_anim_gbc_data[overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (anim_counter)) * 4 + offset] * 16];
    set_bkg_data(first, amount,data);
}

void timer_isr() {
    tick_music();
}

void vblank_isr(){
    if (counter++ == 0) {
        if (current_map == overworld_a_gbc_map) {
            replace_tile(2, 1);
            replace_tile(1, 0);
            replace_tile(SHEET_WIDTH * 3 + 4, 2);
        }else if (current_map == overworld_b_gbc_map) {
            UINT8 indexa = 3;
            UINT8 index = SHEET_WIDTH * 3 + 3;
            replace_tile(SHEET_WIDTH * 3 + 7, indexa);
            replace_tile(index, ++indexa);
            // shore waves
            replace_subtile(++index, ++indexa, 0);
            replace_subtile(index, indexa, 2);
            replace_subtile(++index, ++indexa, 0);
            replace_subtile(index, indexa, 1);
            replace_subtile(++index, ++indexa, 2);
            replace_subtile(index, indexa, 3);
        }
        ++anim_counter;
        anim_counter %= ANIM_WIDTH;
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
        sg->character[0].offset_x = 0;
        sg->character[0].offset_y = 0;

        for(UINT8 i = 0; i < 5; ++i){
            sg->character[i].sprite_index = (i*4);
        }

        sg->lives = 5;
        sg->tpaper = 0;

        sg->chest = 0;
        sg->flame = 0;
        sg->progress[0] = sg->progress[1] = 0;

        sg->magic = 'V';
    }
    counter = 0;
    init_screen();
    init_hud();

    //init_music(&the_journey_begins);
    //init_music(&cosmicgem_voadi);

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
    waitpad_any(J_A | J_START);
    delay(100);
    smart_write(0, 0, 20, 18, strlen(text_youaream), text_youaream);
    waitpad_any(J_A | J_START);
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
