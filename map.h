#ifndef MAP_H
#define MAP_H

#include <gb/gb.h>
const unsigned char * decompress(const UINT8 *compressed_map);
void incject_map_palette(UINT8 x, UINT8 y, UINT8 index);
void incject_map(UINT8 x, UINT8 y, UINT16 index);
void load_map(const UINT8 background[]);
#endif