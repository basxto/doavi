#ifndef UNDICE_H
#define UNDICE_H

void undice_init(UINT8 size, UINT8 *buffer, char *str) NONBANKED;
// unpacks from str to buffer and returns last read char
UINT8 undice_line() NONBANKED;

#endif