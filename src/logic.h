#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>

void teleport_to(const int8_t lx, const int8_t ly, const int8_t px, const int8_t py);
uint8_t move_player(const int8_t x, const int8_t y);
void interact(void);
#endif