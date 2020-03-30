
#ifndef HUD_H
#define HUD_H

#include <gb/gb.h>
#define WIN_START (0)
#define PORTRAIT_START (96)
#define PORTRAIT_LENGTH (16)

void init_hud();
void write_line(UINT8 x, UINT8 y, UINT8 length, char *str);
// maximum length is 2 since maximum UINT8 is FF
void write_hex(UINT8 x, UINT8 y, UINT8 length, UINT8 num);
// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num);
void draw_hud(const UINT8 lives, const UINT8 toiletpaper);

void dialog(UINT8 length, char *str, UINT8 namelength, char* name, UINT8 portrait);

#endif
