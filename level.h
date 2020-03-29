
#ifndef LEVEL_H
#define LEVEL_H

#include <gb/gb.h>

typedef struct {
    const unsigned int *background;
    const unsigned int *sprites;
    const unsigned int *collision;
} Level;

#endif