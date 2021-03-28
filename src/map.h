#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>

const unsigned char * decompress(const uint8_t *compressed_map);
void incject_map_palette(uint8_t x, uint8_t y, uint8_t index);
void incject_map(uint8_t x, uint8_t y, uint16_t index);
void incject_collision(uint8_t x, uint8_t y, bool enable);
void load_map(const uint8_t background[]);
#endif