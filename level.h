
#ifndef LEVEL_H
#define LEVEL_H

#include <gb/gb.h>

typedef struct {
    const UINT8 *background;
    const UINT8 *collision;
    const UINT8 chest;
    const UINT8 flame;
} Level;

#endif
