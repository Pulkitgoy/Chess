#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"

/* Castling rights bitmask */
#define CASTLE_WK  (1 << 0)   /* White kingside  */
#define CASTLE_WQ  (1 << 1)   /* White queenside */
#define CASTLE_BK  (1 << 2)   /* Black kingside  */
#define CASTLE_BQ  (1 << 3)   /* Black queenside */

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
    int ep_square;       /* -1 = none; else the square a pawn may capture to */
    int castling_rights; /* bitfield: CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ */

} Board;

void update_occupancies(Board *board);
void init_start_position(Board *board);
void print_board(Board *board);

#endif