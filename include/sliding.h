#ifndef SLIDING_H
#define SLIDING_H

#include "board.h"
#include "move.h"

void generate_bishop_moves(
    Board *board,
    MoveList *list,
    int side
);
void generate_rook_moves(
    Board *board,
    MoveList *list,
    int side
);
void  generate_queen_moves(
    Board *board,
    MoveList *list,
    int side
);
#endif