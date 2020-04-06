#ifndef LOGIC_H
#define LOGIC_H

#include <gb/gb.h>

void teleport_to(const INT8 lx, const INT8 ly, const INT8 px, const INT8 py);
UINT8 move_player(const INT8 x, const INT8 y, const UINT8 *collision);
void interact();
#endif