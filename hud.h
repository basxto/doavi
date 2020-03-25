
#ifndef HUD_H
#define HUD_H

#include <gb/gb.h>
#define WIN_START (0)

void init_hud();
void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) ;
// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num);
void draw_hud(const UINT8 lives, const UINT8 toiletpaper);

#endif