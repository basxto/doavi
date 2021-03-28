
#ifndef HUD_H
#define HUD_H

#include <stdint.h>
#define WIN_START U8(0)
// offset - \0 and \n
#define FONT_START U8(16-2)
#define PORTRAIT_START U8(96)
#define PORTRAIT_LENGTH U8(16)
#define ITEM_SPRITE U8(24)
#define MOUTH_SPRITE U8(25)
#define MOUTHS_START U8(0x70)

// preloads all compressed images
void preload_hud();
// inits the window
void init_hud();
// fill area with spaces
void space_area(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height);
// write text into an area
// scroll if necessary
uint8_t smart_write(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const char *str);
void write_line(const uint8_t x, const  uint8_t y, const uint8_t length, const char *str);
// maximum length is 2 since maximum uint8_t is FF
void write_hex(const uint8_t x, const uint8_t y, uint8_t length, const uint8_t num);
// maximum length is 3 since maximum uint8_t is 255
void write_num(const uint8_t x, const uint8_t y, uint8_t length, uint8_t num);
void draw_hud(const uint8_t lives, const uint8_t toiletpaper);

uint8_t dialog(const char const *str, const char const *name, const uint8_t mouth);

// like waitpad but looks for any of the given keys
// does not keep cpu busy
void waitpad_any(uint8_t mask);

#endif
