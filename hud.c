#include "hud.h"

void init_hud(){
	unsigned char tiles[1];
	UINT8 x;
	UINT8 y;
	VBK_REG=1;
	tiles[0] = 4;
	for(x = 0; x < 20; ++x){
		for(y = 0; y < 4; ++y){
			set_win_tiles(x,y,1,1,tiles);
		}
	}
	VBK_REG=0;
	tiles[0] = WIN_START+24;
	for(x = 0; x < 20; ++x){
		for(y = 0; y < 4; ++y){
			set_win_tiles(x,y,1,1,tiles);
		}
	}
}

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
	UINT8 buffer[16];
    for (i = 0; i != 16; i++) {
        buffer[i] = WIN_START + ' ';
    }
    for (i = 0; i != length; i++) {
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
    set_win_tiles(x, y, length, 1, buffer);
}

// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num) {
    char buffer[] = "000";
    if (length == 0) {
        return;
    }
    if (length & (~3)) { // >3
        length = 3;
    }
    buffer[2] = '0' + (num % 10);
    num /= 10;
    buffer[1] = '0' + (num % 10);
    num /= 10;
    buffer[0] = '0' + (num % 10);
    num /= 10;
    write_line(x, y, length, buffer + (3 - length));
}

void draw_hud(const UINT8 lives, const UINT8 toiletpaper){
	UINT8 i;
	unsigned char tiles[2];
	tiles[0] = WIN_START+9;
	set_win_tiles(2,1,1,1,tiles);
	tiles[0] = WIN_START+8;
	set_win_tiles(3,1,1,1,tiles);
	tiles[0] = WIN_START+12;
	tiles[1] = WIN_START+13;
	set_win_tiles(0,0,2,1,tiles);
	tiles[0] = WIN_START+14;
	tiles[1] = WIN_START+15;
	set_win_tiles(0,1,2,1,tiles);
	for(i = 0; i < 5; ++i){
		if(i >= lives){
			tiles[0] = WIN_START+11;
		}else{
			tiles[0] = WIN_START+10;
		}
		set_win_tiles(2+i,0,1,1,tiles);
	}
	write_num(4,1,3,toiletpaper);
	move_win(7,16*8);
}