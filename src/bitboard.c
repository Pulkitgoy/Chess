#include "../include/bitboard.h"
#include <stdio.h>


void set_bit(U64 *bb, int square)
{
    *bb |= (1ULL << square);
}

void clear_bit(U64 *bb, int square)
{
    *bb &= ~(1ULL << square);
}

int get_bit(U64 bb, int square)
{
    return (bb >> square) & 1ULL;
}

int get_lsb_index(U64 bb)
{
    if(!bb)
        return -1;

    return __builtin_ctzll(bb);
}

void print_bitboard(U64 bb)
{
    for(int rank = 7; rank >= 0; rank--)
    {
        printf("%d  ", rank + 1);

        for(int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            printf("%d ", get_bit(bb, square));
        }

        printf("\n");
    }

    printf("\n   a b c d e f g h\n\n");
}