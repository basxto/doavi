#ifndef MAP_H
#define MAP_H

#include <gb/gb.h>
#ifdef COMPRESS
const unsigned char * decompress(const UINT8 *compressed_map);
#else
// no compression needed
#define decompress(x) (x)
#endif
void incject_map(UINT8 x, UINT8 y, UINT16 index);
void load_map(const UINT8 background[]);
#endif