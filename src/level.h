
#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>

typedef struct {
    const uint8_t *background;
    const uint8_t *collision;
    const uint8_t chest;
    const uint8_t flame;
} Level;

#endif
