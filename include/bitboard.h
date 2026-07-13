#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>

typedef uint64_t U64;

void set_bit(U64 *bb, int square);
void clear_bit(U64 *bb, int square);
int get_bit(U64 bb, int square);
int get_lsb_index(U64 bb);
void print_bitboard(U64 bb);

#endif