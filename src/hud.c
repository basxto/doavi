#include <string.h>
#include <stdbool.h>
#include "hud.h"
#include "main.h"
#include "../build/strings.h"
#include "../build/pix.h"
#include "../dev/png2gb/csrc/decompress.h"
#include "utils.h"
#include "unpb16.h"
#include "unlz3.h"
#include "undice.h"

extern uint8_t decompressed_tileset[128*16];
// decompress to RAM
uint8_t decompressed_dialog_photos[dialog_photos_data_length*16];

unsigned char *const  dialog_photos[] = {
    &decompressed_dialog_photos[0],
    &decompressed_dialog_photos[1*16*16],
    &decompressed_dialog_photos[2*16*16],
    &decompressed_dialog_photos[3*16*16],
    &decompressed_dialog_photos[4*16*16],
    &decompressed_dialog_photos[5*16*16],
    &decompressed_dialog_photos[6*16*16],
    &decompressed_dialog_photos[7*16*16],
    &decompressed_dialog_photos[8*16*16],
    &decompressed_dialog_photos[9*16*16],
    &decompressed_dialog_photos[10*16*16],
    &decompressed_dialog_photos[11*16*16]
};

#define buffer_length (16)
// be cautious with this!
uint8_t buffer[buffer_length];

void waitpad_any(uint8_t mask){
    while ((joypad() & mask) == 0) {
        wait_vbl_done();
    }
}

void preload_hud() {
    lz3_unpack_block(decompressed_dialog_photos, dialog_photos_data);
    //set_bkg_data(0, 255, &decompressed_dialog_photos[0]);
    //while(1);
    move_sprite(MOUTH_SPRITE, 0x94, 0x91);
    set_sprite_prop(MOUTH_SPRITE, 3);
    set_sprite_tile(MOUTH_SPRITE, MOUTHS_START);
}

void init_hud() {
    unsigned char tiles[1];
    uint8_t x;
    uint8_t y;
    HIDE_WIN;
    VBK_REG = 1;
    tiles[0] = 7;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 20; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + 6;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 20; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    SHOW_WIN;
}

// fill area with spaces
void space_area(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height){
    buffer[0] = (WIN_START - (' '/2)) + '  ';
    for(uint8_t tmp_y = 0; tmp_y < height; ++tmp_y){
        uint8_t tmp = y + tmp_y;
        for(uint8_t tmp_x = 0; tmp_x < width; ++tmp_x){
            set_win_tiles(x + tmp_x, tmp, 1, 1, buffer);
        }
    }
}

// write text into an area
// scroll if necessary
// \1 is new line
// width must be <= buffer_length
uint8_t smart_write(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const char *str){
    uint8_t start = 0;
    uint8_t run = 1;
    uint8_t tmp_y = y;
    uint8_t max = 0;
    uint8_t choices = 0;
    uint8_t firstchoice = y;
    uint8_t jump_back = 0;
    // string return pointer
    char *str_ret = 0;
    // string pointer
    const char *str_ptr = str;
    space_area(x, y, width, height);
    undice_init(width, buffer, str);
    //undice_line();
    while(run != 0){
        // stop on null char
        run = undice_line();
        if(buffer[0] == specialchar_2){
            if(choices++ == 0)
                firstchoice = tmp_y;
            buffer[0] = specialchar_1;
        }
        write_line(x, tmp_y, width, buffer);
        ++tmp_y;
        // scroll when we reach the end
        if(tmp_y >= (uint8_t)(y+height) && run){
            buffer[0] = specialchar_3;
            write_line(x+width-1, tmp_y-1, 1, buffer);
            delay(100);
            waitpad_any(J_A);
            delay(100);
            tmp_y = y;
            space_area(x, y, width, height);
        }
    }
    // let user select
    if(choices != 0){
        tmp_y = firstchoice;
        //write arrow
        buffer[buffer_length - 1] = specialchar_2;
        write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
        // terminates with return
        while(true){
            delay(100);
            buffer[buffer_length - 1] = specialchar_1;
            write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
            switch(joypad()){
                case J_UP:
                    if(tmp_y > firstchoice)
                        --tmp_y;
                    break;
                case J_DOWN:
                    if(tmp_y < U8(firstchoice + choices - 1))
                        ++tmp_y;
                    break;
                case J_A:
                    return (tmp_y - firstchoice) + 1;
                    break;
            }
            buffer[buffer_length - 1] = specialchar_2;
            write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
        }
    }
    return 0;
}

void write_line(const uint8_t x, const  uint8_t y, const uint8_t length, const char *str) {
    if(length == 0)
        return;
    uint8_t i;
    for (i = 0; i < length; ++i) {
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        // lower case is treated as special characters
        buffer[i] =  FONT_START + str[i];
    }
    for (; i < length; ++i) {
        buffer[i] = FONT_START + specialchar_1;
    }
    set_win_tiles(x, y, length, 1, buffer);
}

// maximum length is 2 since maximum uint8_t is FF
void write_hex(const uint8_t x, const uint8_t y, uint8_t length, const uint8_t num) {
    if (length > 2) {
        length = 2;
    }
    buffer[buffer_length-1] = '\0';
    buffer[buffer_length-2] = specialchar_4 + (num & 0xf);
    buffer[buffer_length-3] = specialchar_4 + (num >> 4);
    write_line(x, y, length, buffer + U8(U8(buffer_length - 1) - length));
}

// maximum length is 3 since maximum uint8_t is 255
void write_num(const uint8_t x, const uint8_t y, uint8_t length, uint8_t num) {
    if (length > 3) {
        length = 3;
    }
    buffer[buffer_length-1] = '\0';
    //put three numbers into buffer (right to left)
    for(uint8_t i = buffer_length-2; i > buffer_length-6; --i){
        buffer[i] = specialchar_4 + U8(num % 10);
        num /= U8(10);
    }
    write_line(x, y, length, buffer + U8(U8(buffer_length - 1) - length));
}

void draw_hud(const uint8_t lives, const uint8_t toiletpaper) {
    uint8_t i;
    unsigned char tiles[2];
    uint8_t item = get_selected_item();
    // clear it
    tiles[1] = tiles[0] = WIN_START + 6;
    for(i = 0;i < 20; ++i)
        set_win_tiles(i, 0, 1, 2, tiles);
    // draw decoration
    tiles[0] = WIN_START + 7;
    set_win_tiles(2, 1, 1, 1, tiles);
    tiles[0] = WIN_START + 8;
    set_win_tiles(3, 1, 1, 1, tiles);
    tiles[0] = WIN_START + 12;
    tiles[1] = WIN_START + 13;
    set_win_tiles(0, 0, 2, 1, tiles);
    tiles[0] = WIN_START + 14;
    tiles[1] = WIN_START + 15;
    set_win_tiles(0, 1, 2, 1, tiles);
    for (i = 5; i > 0; ++i) {
        tiles[0] = (i > lives ? WIN_START + U8(11) : WIN_START + U8(9));
        set_win_tiles(1 + i, 0, 1, 1, tiles);
    }
    // write values
    write_num(4, 1, 3, toiletpaper);
    move_win(7, 16 * 8);
    move_sprite(ITEM_SPRITE, 12, 148);
    set_sprite_prop(ITEM_SPRITE, 3);
    set_sprite_tile(ITEM_SPRITE, ITEMS_START-1+item);
    set_sprite_tile(MOUTH_SPRITE, MOUTHS_START);
}

uint8_t dialog(const char const *str, const char const *name, const uint8_t mouth){
    uint8_t portrait = 0;
    unsigned char tiles[16];
    uint8_t x;
    uint8_t y;
    uint8_t accept = 0;
    uint8_t ret = 0;
    uint8_t namelength = 0;

    undice_init(buffer_length, buffer, name);
    undice_line();

    if (name == text_sign)
        portrait = 1;
    if (name == text_grave)
        portrait = 2;
    if (name == text_flame)
        portrait = 3;
    if (name == text_letter)
        portrait = 4;
    if (name == text_ghost)
        portrait = 6;
    if (name == text_t0)
        portrait = 7;
    if (name == text_t1)
        portrait = 8;
    if (name == text_rachel)
        portrait = 11;

    // generate name field data blocks
    // there is room for at most 13 characters
    for(y = 0; y < 13; ++y){
        if(!buffer[y])
            break;
        get_bkg_data(FONT_START + buffer[y], 1, tiles);
        // generate lower case
        if(y!=0)
            for(x = 9; x > 1; --x){
                tiles[x] = tiles[x-2];
            }
        // lines at top and bottom
        tiles[0] = 0xFF;
        tiles[14] = 0xFF;

        for(x = 0; x < 16; ++x){
            tiles[x] = tiles[x] | (tiles[x]>>1);
            // invert white to brown and not black
            if((x%2)==1){
                tiles[x] = ~(tiles[x]);
            }
        }
        set_win_data(PORTRAIT_START + PORTRAIT_LENGTH + y, 1, tiles);
        ++namelength;
    }

    //set brown
    VBK_REG = 1;
    tiles[0] = 7;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;

    //tiles[0] = 2 is already set
    // set portrait
    set_bkg_data(PORTRAIT_START, PORTRAIT_LENGTH, dialog_photos[portrait]);
    set_sprite_tile(MOUTH_SPRITE, MOUTHS_START+mouth);


    // hide item
    move_sprite(ITEM_SPRITE, 0, 0);
    // 2 or 3
    if(U8(portrait & 0x3) != 0){
        tiles[0] = portrait+1;
    }

    // set portrait color
    VBK_REG = 1;
    for (x = 20-4; x < 4+20-4; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;

    // draw portrait
    //#define PORTRAIT_LENGTH (16)
    //#define PORTRAIT_START (300)
    tiles[0] = PORTRAIT_START;
    for (x = 20-4; x < 4+20-4; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
            tiles[0]++;
        }
    }

    //top line
    tiles[0] = WIN_START + 2;
    for (x = 1; x < 20-1-4; ++x) {
        set_win_tiles(x, 0, 1, 1, tiles);
    }

    //set_win_data(PORTRAIT_START + PORTRAIT_LENGTH, 1, tiles);
    // write name field
    for(y = 0; y < namelength; ++y){
        tiles[0] = PORTRAIT_START + PORTRAIT_LENGTH + y;
        set_win_tiles(1 + y, 0, 1, 1, tiles);
    }
    tiles[0] = WIN_START + 1;
    set_win_tiles(1 + namelength , 0, 1, 1, tiles);

    // line left and right
    tiles[0] = WIN_START + 4;
    for (y = 3; y > 0; --y) {
        set_win_tiles(0, y, 1, 1, tiles);
        tiles[0]++;
        set_win_tiles(20-1-4, y, 1, 1, tiles);
        tiles[0]--;
    }

    tiles[0] = WIN_START;
    set_win_tiles(0, 0, 1, 1, tiles);
    tiles[0] = WIN_START + 3;
    set_win_tiles(20-1-4, 0, 1, 1, tiles);

    //write_line(1, 0, namelength, name);

    move_win(7, 14 * 8);
    ret = smart_write(1, 1, 14, 3, str); // does smart_write need to change this?
    delay(100);
    // only wait for A if this isn't a selection
    if(ret == 0){
        waitpad_any(J_A);
        delay(100);
    }
    init_hud();
    return ret;
}