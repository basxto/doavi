#include "hud.h"
#include "pix/dialog_photos_data.c"

// as defined in makefile
//const unsigned char * win_gbc_data_inrom;// = (0x7FFF-0x1880);
extern const unsigned char win_gbc_data[];

#define buffer_length (16)
// be cautious with this!
UINT8 buffer[buffer_length];

void init_hud() {
    unsigned char tiles[1];
    UINT8 x;
    UINT8 y;
    HIDE_WIN;
    VBK_REG = 1;
    tiles[0] = 2;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 20; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    VBK_REG = 0;
    tiles[0] = WIN_START + (' '-8);
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 20; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    SHOW_WIN;
}

// fill area with spaces
void space_area(const UINT8 x, const UINT8 y, const UINT8 width, const UINT8 height){
    buffer[0] = WIN_START + ' ';
    for(UINT8 tmp_y = 0; tmp_y < height; ++tmp_y){
        for(UINT8 tmp_x = 0; tmp_x < width; ++tmp_x){
            set_win_tiles(x + tmp_x, y + tmp_y, 1, 1, buffer);
        }
    }
}

// write text into an area
// scroll if necessary
UINT8 smart_write(const UINT8 x, const UINT8 y, const UINT8 width, const UINT8 height, UINT8 length, char *str){
    UINT8 start = 0;
    UINT8 end = 0;
    UINT8 run = 1;
    UINT8 tmp_y = y;
    UINT8 max = 0;
    UINT8 choices = 0;
    UINT8 firstchoice = y;
    space_area(x, y, width, height);
    while(run){
        // detect choices
        if(start == 0 && str[start] == '>'){
            if(choices++ == 0)
                firstchoice = tmp_y;
            buffer[buffer_length - 1] = ' ';
            write_line(x + start, tmp_y, 1, buffer + (buffer_length - 1));
            start += 1;
        }
        // regular stuff
        max = start + 16;
        if(width < max)
            max = width;
        if(length < max)
            max = length;
        for(; end < max; ++end)
            //get to that next round
            //end of this line
            if(str[end] == '\n' || str[end] == '\0')
                break;

        write_line(x + start, tmp_y, (end-start), str + start);
        start = end;

        if(str[start] == '\0' || start >= length){
            run = 0;
        } else {
            if(str[start] == '\n')
                ++start;//skip
            length -= start;
            str += start;
            start = 0;
            end = 0;
            tmp_y += 1;
        }
        if(tmp_y >= y+height){
            // if it reached the width, we overwite the last letter
            if(str[start-1] != '\n')
                --str;
                ++length;
            buffer[0] = WIN_START + 7;
            buffer[buffer_length - 1] = 7;
            write_line(x + width - 1, y + height - 1, 1, buffer + (buffer_length - 1));
            delay(100);
            waitpad(J_A);
            delay(100);
            tmp_y = y;
            space_area(x, y, width, height);
        }
        
    }
    // let user select
    if(choices != 0){
        tmp_y = firstchoice;
        run = 1;
        //write arrow
        buffer[buffer_length - 1] = 6;
        write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
        while(run){
            delay(100);
            buffer[buffer_length - 1] = ' ';
            write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
            switch(joypad()){
                case J_UP:
                    if(tmp_y > firstchoice)
                        --tmp_y;
                    break;
                case J_DOWN:
                    if(tmp_y < firstchoice + choices - 1)
                        ++tmp_y;
                    break;
                case J_A:
                    return (tmp_y - firstchoice) + 1;
                    break;
            }
            buffer[buffer_length - 1] = 6;
            write_line(x, tmp_y, 1, buffer + (buffer_length - 1));
        }
    }
    return 0;
}

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
    if(length == 0)
        return;
    for (i = 0; i != length; i++) {
        buffer[i] = WIN_START + ' ';
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        // we handle lower case in converter script
        buffer[i] = WIN_START + (str[i]);
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

UINT8 dialog(UINT8 length, const char *str, UINT8 namelength, const char* name, UINT8 portrait){
    unsigned char tiles[16];
    UINT8 x;
    UINT8 y;
    UINT8 accept = 0;
    UINT8 ret = 0;

    //set brown
    VBK_REG = 1;
    tiles[0] = 2;
    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 4; ++y) {
            set_win_tiles(x, y, 1, 1, tiles);
        }
    }
    /*tiles[0] = 5;
    for (x = 0; x < namelength; ++x) {
        set_win_tiles(1 + x, 0, 1, 1, tiles);
    }*/
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

    // generate name field data blocks
    // line at bottom and top
    tiles[0] = tiles[1] = 0xFF;
    tiles[14] = tiles[15] = 0xFF;
    for(y = 0; y < namelength; ++y){
       for(x = 2; x < 14; ++x){
           tiles[x] = ~win_gbc_data[(name[y])*16 + x];
       }
       set_win_data(PORTRAIT_START + PORTRAIT_LENGTH + y, 1, tiles);
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
    for (y = 1; y < 4; ++y) {
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
    ret = smart_write(1, 1, 14, 3, length, str);
    delay(100);
    // only wait for A if this isn't a selection
    if(ret == 0){
        waitpad(J_A);
        delay(100);
    }
    init_hud();
    return ret;
}