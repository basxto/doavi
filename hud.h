
#ifndef HUD_H
#define HUD_H

#include <gb/gb.h>
#define WIN_START $(0)
// offset - \0 and \n
#define FONT_START $(16-2)
#define PORTRAIT_START $(96)
#define PORTRAIT_LENGTH $(16)
#define ITEM_SPRITE $(24)
#define MOUTH_SPRITE $(25)
#define MOUTHS_START $(0x70)

// preloads all compressed images
void preload_hud();
// inits the window
void init_hud();
// fill area with spaces
void space_area(const UINT8 x, const UINT8 y, const UINT8 width, const UINT8 height);
// write text into an area
// scroll if necessary
UINT8 smart_write(const UINT8 x, const UINT8 y, const UINT8 width, const UINT8 height, char *str);
void write_line(UINT8 x, UINT8 y, UINT8 length, char *str);
// maximum length is 2 since maximum UINT8 is FF
void write_hex(UINT8 x, UINT8 y, UINT8 length, UINT8 num);
// maximum length is 3 since maximum UINT8 is 255
void write_num(UINT8 x, UINT8 y, UINT8 length, UINT8 num);
void draw_hud(const UINT8 lives, const UINT8 toiletpaper);

UINT8 dialog(const char const *str, const char const *name, const UINT8 mouth);

// like waitpad but looks for any of the given keys
// does not keep cpu busy
void waitpad_any(UINT8 mask);

#endif
