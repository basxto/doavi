// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <gb/hardware.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

#include "hud.h"
#include "logic.h"
#include "map.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"

#include "pix/pix.h"

#include "strings.h"

#include "unpb16.h"
#include "unlz3.h"

#include "level.h"

extern const Level level[][7];

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];

//TODO: pretty much a hack
extern UINT8 decompressed_tileset[128*16];

// for scanline effects
const UINT8 scanline_wobble[] = {4,5,6,7,7,6,5,4,4,5,6,7,7,6,5};
const UINT8 *scanline_offsets;
UINT8 scanline_ly_offset;

UINT8 counter;
UINT8 anim_counter;

// since might need to decompress it
const UINT8 *current_background;
// since we want to modify it
UINT8 current_collision[10];
UINT8 current_chest;
UINT8 current_flame;

extern const unsigned char *current_map;
Saveslots *sl = (Saveslots *)0xa000;
Savegame *sg;
Savegame sgemu;

UINT8 level_x;
UINT8 level_y;
UINT8 lives;
UINT8 tpaper;
Character character[5];
UINT8 item[8];
UINT8 selected_item;
UINT8 chest;
UINT8 flame;
UINT8 progress[2];
_Bool cheat;

void efficient_delay(UINT8 time){
    for(; time != 0; --time)
        wait_vbl_done();
}

// reads values from game header
// for easy game genie support
void init_save() {
    sg->level_x = 1;//(*(volatile UINT8*)0x13D)&0xF;// 1
    sg->level_y = 4;//(*(volatile UINT8*)0x13A)&0xF;// 4
    //memcpy(sg->name, "candyhead", 10);
    sg->x = 4;//(*(volatile UINT8*)0x13B)>>4; // 4
    sg->y = 4;//(*(volatile UINT8*)0x13C)>>4; // 4

    for(UINT8 i = 0; i < 8; ++i){
        sg->item[i] = 0;
    }
    sg->selected_item = 0;

    sg->lives = 4;//(*(volatile UINT8*)0x135)&0xF; // 4
    sg->tpaper = 0;//*(volatile UINT8*)0x142; // 0

    sg->chest = 0;
    sg->flame = 0;
    sg->progress[0] = 0x40;//*(volatile UINT8*)0x144; // 0
    sg->progress[1] = 0;//*(volatile UINT8*)0x145; // 0
    sg->cheat = 0;//*(volatile UINT8*)0x146; // 0
}

void load_menu() {
    smart_write(0, 0, 20, 18, text_load_title);
    UINT8 save = smart_write(0, 4, 5, 18, text_load_select) - 1;
    sg = (Savegame *)(0xa000 + sizeof(Saveslots) + (save*sizeof(Savegame)));
    //delay(100);
    ENABLE_RAM_MBC1;
    if((sl->slots & (0x1<<save)) == 0){ // new save
        init_save();
        sl->slots |= (0x1<<save);
    }else{ // existing one
        DISABLE_RAM_MBC1;
        UINT8 ret = smart_write(0, 0, 20, 18, text_load_choice);
        ENABLE_RAM_MBC1;
        if(ret == 2){
            init_save();
            sl->slots |= (0x1<<save);
        }
    }
    DISABLE_RAM_MBC1;
    load_sg();
    change_level();
}

void item_menu(){
    smart_write(0, 0, 20, 18, text_item_selection);
    if(item[0] != 0)
        smart_write(5, 4, 16, 1, text_sword);
    if(item[1] != 0)
        smart_write(5, 5, 16, 1, text_power);
    UINT8 item = smart_write(0, 4, 5, 18, text_load_select) - 1;
    if(item != 0)
        selected_item = item-1;
}

void menu() {
    move_win(7, 0);
    HIDE_SPRITES;
    UINT8 ret = smart_write(0, 0, 20, 18, text_menu);
    // write_num(12, 1, 3, ret);
    switch (ret) {
    case 2:
        smart_write(0, 0, 20, 18, text_credits);
        waitpad_any(J_A);
        delay(100);
        break;
    case 4:
        load_menu();
        break;
    case 5:
        item_menu();
        break;
    }
    draw_hud(lives, tpaper);
    SHOW_SPRITES;
}

void scanline_isr() {
    SCX_REG = scanline_offsets[LY_REG%8];
}

void screen_wobble() {
    scanline_ly_offset = 0;
    scanline_offsets = &scanline_wobble[0];

    set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
    for(UINT8 i = 40; i > 0; --i){
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        scanline_offsets = &scanline_wobble[$(++scanline_ly_offset)%8];
    }
    set_interrupts(VBL_IFLAG | TIM_IFLAG);
    SCX_REG = 8;
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
    pb16_unpack_win_data(WIN_START, win_gbc_data_length, decompressed_tileset, win_gbc_data);
    pb16_unpack_sprite_data(ITEMS_START, items_gbc_data_length, decompressed_tileset, items_gbc_data);
    pb16_unpack_sprite_data(CHARACTERS_START, modular_characters_data_length, decompressed_tileset, modular_characters_data);
    lz3_unpack_sprite_data(MOUTHS_START, dialog_mouths_data_length, decompressed_tileset, dialog_mouths_data);
    preload_hud();

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

    // switch to different time
    if(IS_PRGRS_TIME(1)){
        if(level_y == 0 && level_x == 0)
            level_x = 6;
    }
    if(IS_PRGRS_TIME(2)){
        if(level_y == 1 && level_x == 4)
            level_x = 6;
    }

    // load level into ram
    memcpy(current_collision, level[level_y][level_x].collision, 10);
    current_background = decompress(level[level_y][level_x].background);
    current_chest = level[level_y][level_x].chest;
    current_flame = level[level_y][level_x].flame;
    for(UINT8 i = 1; i < 5; ++i){
        // disable characters
        character[i].sprite = 0xFF;
        character[i].offset_x = 0;
        character[i].offset_y = 0;
        character[i].direction = 0;
        render_character(i);
    }
    save_sg();
    load_map(current_background);
}

UINT8 get_selected_item(){
    return item[selected_item];
}

// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const UINT8 index) {
    Character *chrctr = &character[index];
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
        UINT8 base = (10*8) - 7 + (chrctr->sprite*19) + ((chrctr->direction&0x03)*4);
        UINT8 palette = chrctr->palette&0x0F;
        for(UINT8 i = 0; i < 2; ++i){
            mapping = modular_characters_map[base];
            set_sprite_tile(++index, CHARACTERS_START+(mapping&0x7F));
            // 0x80 marks mirrored tiles
            set_sprite_prop(index, palette | (mapping&0x80? $(0x20) : $(0x0) ));
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
            set_sprite_prop(index, palette | (mapping&0x80? $(0x20) : $(0x0) ));
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
            if(character[i].sprite != 0xFF && character[i].x == x && character[i].y == y)
                return 0;
        return 1;
    }
    return 0;
}

UINT8 move_character(const UINT8 index, const INT8 x, const INT8 y) {
    Character *chrctr = &character[index];
    if (chrctr->x == 0 && x < 0) {
        level_x--;
        chrctr->x += WIDTH + x;
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        level_x++;
        chrctr->x += -WIDTH + x;
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        level_y--;
        chrctr->y += HEIGHT + y;
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        level_y++;
        chrctr->y += -HEIGHT + y;
        change_level();
        return 0;
    }
    //write_num(12, 1, 3, tile);
    if (is_free((chrctr->x + x),(chrctr->y + y)) != 0) {
        chrctr->offset_x = x * $(-16);
        chrctr->offset_y = y * $(-16);
        chrctr->x += x;
        chrctr->y += y;
        render_character(index);
        while(chrctr->offset_x != 0 || chrctr->offset_y != 0){
            wait_vbl_done();
        }
        return 0;
    } else {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }
}

// index of tile in spritesheet; index of tile in animation sheet
// 16x16 block indices
void replace_tile(UINT8 index, UINT8 indexa){
    UINT8 first = SHEET_START + current_map[$(index*4)];
    UINT8 amount = 4;
    const unsigned char *data = &overworld_anim_gbc_data[overworld_anim_gbc_map[$($($(indexa*ANIM_WIDTH) + (anim_counter)) * 4)] * 16];
    set_bkg_data(first, amount,data);
}

// for deduplicated tiles
void replace_subtile(UINT8 index, UINT8 indexa, UINT8 offset){
    UINT8 first = SHEET_START + current_map[$($(index*4) + offset)];
    UINT8 amount = 1;
    const unsigned char *data = &overworld_anim_gbc_data[overworld_anim_gbc_map[$($($(indexa*ANIM_WIDTH) + (anim_counter)) * 4 + offset)] * 16];
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
            replace_tile(SHEET_WIDTH * 3 + 7, 2);
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
    if ((counter % $(4)) == 0){

        // move characters
        for(UINT8 i = 0; i < 5; ++i){
            _Bool changed = 0;
            Character *chrctr = &character[i];
            // move character
            if(chrctr->offset_x > 0){
                chrctr->offset_x -= 4;
                changed = 1;
            }
            if(chrctr->offset_x < 0){
                chrctr->offset_x += 4;
                changed = 1;
            }
            if(chrctr->offset_y > 0){
                chrctr->offset_y -= 4;
                changed = 1;
            }
            if(chrctr->offset_y < 0){
                chrctr->offset_y += 4;
                changed = 1;
            }

            // pretty much a short version of render_character
            // only render position
            if(changed && chrctr->sprite != 0xFF){
                UINT8 index = chrctr->sprite_index-1;
                UINT8 x = $(8) + (chrctr->x) * $(16) + chrctr->offset_x;
                UINT8 y = ((chrctr->y) + $(1)) * $(16) + chrctr->offset_y - $(2);
                move_sprite(++index, x, y);
                x+=8;
                move_sprite(++index, x, y);
                x-=8;
                y+=8;
                move_sprite(++index, x, y);
                x+=8;
                move_sprite(++index, x, y);
            }
        }
    }
    counter %= $(32);
}

void save_sg(){
    ENABLE_RAM_MBC1;
    sg->level_x = level_x;
    sg->level_y = level_y;
    sg->lives = lives;
    sg->tpaper = tpaper;
    sg->selected_item = selected_item;
    sg->chest = chest;
    sg->flame = flame;
    sg->progress[0] = progress[0];
    sg->progress[1] = progress[1];
    sg->cheat = cheat;
    sg->x = character[0].x;
    sg->y = character[0].y;
    sg->direction = character[0].direction;
    memcpy(sg->item, item, 8);
    if(cheat)
        memcpy(sg->name, "cheathead", 10);
    DISABLE_RAM_MBC1;
}

void load_sg(){
    ENABLE_RAM_MBC1;
    level_x = sg->level_x;
    level_y = sg->level_y;
    lives = sg->lives;
    tpaper = sg->tpaper;
    selected_item = sg->selected_item;
    chest = sg->chest;
    flame = sg->flame;
    progress[0] = sg->progress[0];
    progress[1] = sg->progress[1];
    cheat = sg->cheat;
    character[0].x = sg->x;
    character[0].y = sg->y;
    character[0].direction = sg->direction;
    memcpy(item, sg->item, 8);
    DISABLE_RAM_MBC1;
    character[0].offset_x = 0;
    character[0].offset_y = 0;
}

void main() {
    sg = (Savegame *)(0xa000 + sizeof(Saveslots) + (0*sizeof(Savegame)));


    // initialize main character
    character[0].sprite = 0;
    character[0].palette = (2<<4)|2;
    // initialize sprites for characters
    for(UINT8 i = 0; i < 5; ++i){
        character[i].sprite_index = (i*4);
    }
    //sg = &sgemu;
    // load savegame
    ENABLE_RAM_MBC1;
    SWITCH_RAM_MBC1(0);
    if(sl->magic != 'V') {
        sl->magic = 'V';
        sl->slots = 0x0;
    }
    DISABLE_RAM_MBC1;
    counter = 0;
    init_screen();
    init_hud();

    SHOW_BKG;
    SHOW_WIN;
    DISPLAY_ON;

    // configure interrupt
    // triggers interrupt when it reaches 0xFF
    TMA_REG = TIMA_REG = 0xE3;
    TAC_REG = 0x4 | 0x0; // 4096 Hz

    // configure scanline interrupts
    STAT_REG = 0x18;
    LYC_REG = 0x00;

    // enable interrupts
    disable_interrupts();
    add_TIM(timer_isr);
    add_VBL(vblank_isr);
    add_LCD(scanline_isr);
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

    smart_write(3, 4, 20, 2, text_title);
    smart_write(5, 12, 20, 2, text_author);
    waitpad_any(J_A | J_START);
    delay(100);
    smart_write(0, 0, 20, 18, text_intro);
    waitpad_any(J_A | J_START);
    delay(100);
    current_map = 0x0;
    load_menu();
    init_hud();
    draw_hud(lives, tpaper);

    SHOW_SPRITES;

    while (1) {

        switch (joypad()) {
        case J_RIGHT: // If joypad() is equal to RIGHT
            character[0].direction = 3;
            if (move_player(1, 0) == 1)
                render_character(0);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            character[0].direction = 1;
            if (move_player(-1, 0) == 1)
                render_character(0);
            break;
        case J_UP: // If joypad() is equal to UP
            character[0].direction = 2;
            if (move_player(0, -1) == 1)
                render_character(0);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            character[0].direction = 0;
            if (move_player(0, 1) == 1)
                render_character(0);
            break;
        case J_A:
            interact();
            efficient_delay(15);
            break;
        case J_B:
            if(++selected_item >= 4)
                selected_item = 0;
            draw_hud(lives, tpaper);
            efficient_delay(15);
            break;
        case J_START:
            menu();
            break;
        default:
            break;
        }
        wait_vbl_done();
    }
}
