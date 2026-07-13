#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"

void generate_moves(
    Board *board,
    MoveList *list
);

void generate_pawn_moves(
    Board *board,
    MoveList *list,
    int side    
);
void generate_knight_moves(
    Board *board,
    MoveList *list,
    int side
);
#endif