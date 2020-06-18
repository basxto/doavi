#ifndef UNPACKB16_H
#define UNPACKB16_H

// copies packets*2*8byte from src to dest
void pb16_unpack_block(unsigned char packets, unsigned char* dst, unsigned char* src) NONBANKED;
//void pb16_unpack_bkg_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dest, unsigned char* src) NONBANKED;
void pb16_unpack_bkg_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;
void pb16_unpack_win_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;
void pb16_unpack_sprite_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;

#endif