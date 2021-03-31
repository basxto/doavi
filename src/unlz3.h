#ifndef UNLZ3_H
#define UNLZ3_H

#include <stdint.h>

// unpacks from src to dest
void lz3_unpack_block(unsigned char* dst, const unsigned char* src) __nonbanked;

void lz3_unpack_bkg_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;
void lz3_unpack_win_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;
void lz3_unpack_sprite_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;

#endif