#ifndef UNLZ3_H
#define UNLZ3_H

// unpacks from src to dest
void lz3_unpack_block(unsigned char* dst, unsigned char* src) NONBANKED;

void lz3_unpack_bkg_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;
void lz3_unpack_win_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;
void lz3_unpack_sprite_data(UINT8 first_tile, UINT8 nb_tiles, unsigned char *dst, unsigned char* src) NONBANKED;

#endif