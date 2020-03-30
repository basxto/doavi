#include "hud.h"
#include "pix/dialog_photos_data.c"

#define buffer_length (16)
UINT8 buffer[buffer_length];

#define PORTRAIT_LENGTH (16)
#define PORTRAIT_START (300)

void init_hud() {
    unsigned char tiles[1];
    UINT8 x;
    UINT8 y;
    VBK_REG = 1;
    tiles[0] = 4;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + (' '-8);
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
}

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
    for (i = 0; i != length; i++) {
        buffer[i] = WIN_START + ' ';
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        if (str[i] > 0x20 && str[i] < 0x60) {
            buffer[i] = WIN_START + (str[i]);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            buffer[i] = WIN_START + (str[i] - 0x20);
        } else {
            // everything else, including space, becomes a space
            buffer[i] = WIN_START + ' ';
        }
    }
    for (; i != buffer_length; i++) {
        buffer[i] = WIN_START + ' ';
    }
    set_win_tiles(x, y, length, 1, buffer);
}

const char hex_char[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                         '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// maximum length is 2 since maximum UINT8 is FF
void write_hex(UINT8 x, UINT8 y, UINT8 length, UINT8 num) {
    buffer[buffer_length - 2] = buffer[buffer_length - 1] = '0';
    if (length == 0) {
        return;
    }
    if (length > 2) { // >3
        length = 2;
    }
    buffer[buffer_length - 1] = hex_char[num % 16];
    num /= 16;
    buffer[buffer_length - 2] = hex_char[num % 16];
    num /= 16;
    write_line(x, y, length, buffer + (buffer_length - length));
}

// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num) {
    buffer[buffer_length - 3] = buffer[buffer_length - 2] =
        buffer[buffer_length - 1] = '0';
    if (length == 0) {
        return;
    }
    if (length & (~3)) { // >3
        length = 3;
    }
    buffer[buffer_length - 1] = '0' + (num % 10);
    num /= 10;
    buffer[buffer_length - 2] = '0' + (num % 10);
    num /= 10;
    buffer[buffer_length - 3] = '0' + (num % 10);
    num /= 10;
    write_line(x, y, length, buffer + (buffer_length - length));
}

void draw_hud(const UINT8 lives, const UINT8 toiletpaper) {
    UINT8 i;
    unsigned char tiles[2];
    tiles[0] = WIN_START + 9;
    set_win_tiles(2, 1, 1, 1, tiles);
    tiles[0] = WIN_START + 8;
    set_win_tiles(3, 1, 1, 1, tiles);
    tiles[0] = WIN_START + 12;
    tiles[1] = WIN_START + 13;
    set_win_tiles(0, 0, 2, 1, tiles);
    tiles[0] = WIN_START + 14;
    tiles[1] = WIN_START + 15;
    set_win_tiles(0, 1, 2, 1, tiles);
    for (i = 0; i < 5; ++i) {
        if (i >= lives) {
            tiles[0] = WIN_START + 11;
        } else {
            tiles[0] = WIN_START + 10;
        }
        set_win_tiles(2 + i, 0, 1, 1, tiles);
    }
    write_num(4, 1, 3, toiletpaper);
    move_win(7, 16 * 8);
}

void dialog(UINT8 length, char *str, UINT8 namelength, char* name, UINT8 portrait){
    unsigned char tiles[1];
    UINT8 x;
    UINT8 y;
    UINT8 accept = 0;

    //set brown
    VBK_REG = 1;
    tiles[0] = 2;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    tiles[0] = 5;
    for (x = 0; x < namelength; ++x) {
        set_win_tiles(1 + x, 0, 1, 1, tiles);
    }
    VBK_REG = 0;

    tiles[0] = 2;
    // set portrait
    if(portrait == 0){
        for (x = 0; x < PORTRAIT_LENGTH; ++x) {
            set_win_data(PORTRAIT_START + x, 1, WIN_START + ' ');
        }
    }else{
        set_win_data(PORTRAIT_START, PORTRAIT_LENGTH, dialog_photos_data + ((portrait-1)*16*PORTRAIT_LENGTH));
        if(portrait == 2){
            tiles[0] = 3;
        }
        if(portrait == 3){
            tiles[0] = 4;
        }
    }

    // set portrait color
    VBK_REG = 1;
    for (x = 0; x < 4; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(20-4+x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;

    // draw portrait
    //#define PORTRAIT_LENGTH (16)
    //#define PORTRAIT_START (300)
    tiles[0] = PORTRAIT_START;
    for (x = 0; x < 4; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(20-4+x, y, 1, 1, tiles);
            tiles[0]++;
        }
    }

    //top line
    tiles[0] = WIN_START + 2;
    for (x = 1; x < 20-1-4; ++x) {
        set_win_tiles(x, 0, 1, 1, tiles);
    }

    // line left and right
    tiles[0] = WIN_START + 4;
    for (y = 1; y < 4; ++y) {
        set_win_tiles(0, y, 1, 1, tiles);
        tiles[0]++;
        set_win_tiles(20-1-4, y, 1, 1, tiles);
        tiles[0]--;
    }

    //clear rest
    tiles[0] = WIN_START + ' ';
    for (x = 1; x < 20-1-4; ++x) {
        for (y = 1; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }

    tiles[0] = WIN_START;
    set_win_tiles(0, 0, 1, 1, tiles);
    tiles[0] = WIN_START + 3;
    set_win_tiles(20-1-4, 0, 1, 1, tiles);
    tiles[0] = WIN_START + 1;
    set_win_tiles(1+namelength, 0, 1, 1, tiles);

    write_line(1, 0, namelength, name);
    write_line(1, 1, length, str);
    move_win(7, 14 * 8);
    delay(100);
    while (accept==0) {
        switch (joypad()) {
        case J_A:
            accept = 1;
            break;
        default:
            break;
        }
    }
    init_hud();
}