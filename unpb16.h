#ifndef UNPACKB16_H
#define UNPACKB16_H

// copies packets*2*8byte from src to dest
void pb16_unpack_block(unsigned char packets, unsigned char* src, unsigned char* dest) NONBANKED;

#endif