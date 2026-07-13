#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"

typedef struct
{
    U64 white_pawns;
    U64 white_knights;
    U64 white_bishops;
    U64 white_rooks;
    U64 white_queens;
    U64 white_king;

    U64 black_pawns;
    U64 black_knights;
    U64 black_bishops;
    U64 black_rooks;
    U64 black_queens;
    U64 black_king;
    
    U64 white_occ;
    U64 black_occ;
    U64 all_occ;
    
    int side_to_move;

} Board;

void update_occupancies(Board *board);
void init_start_position(Board *board);
void print_board(Board *board);

#endif