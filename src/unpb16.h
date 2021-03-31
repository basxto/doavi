#ifndef UNPACKB16_H
#define UNPACKB16_H

#include <stdint.h>

// copies packets*2*8byte from src to dest
void pb16_unpack_block(unsigned char packets, unsigned char* dst, const unsigned char* src) __nonbanked;
//void pb16_unpack_bkg_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dest, const unsigned char* src) __nonbanked;
void pb16_unpack_bkg_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;
void pb16_unpack_win_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;
void pb16_unpack_sprite_data(uint8_t first_tile, uint8_t nb_tiles, unsigned char *dst, const unsigned char* src) __nonbanked;

#endif