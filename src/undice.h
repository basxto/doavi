#ifndef UNDICE_H
#define UNDICE_H

#include <stdint.h>

void undice_init(uint8_t size, char *buffer, const char *str) NONBANKED;
// unpacks from str to buffer and returns last read char
uint8_t undice_line() NONBANKED;

#endif