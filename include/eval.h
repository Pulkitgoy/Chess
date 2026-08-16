#ifndef EVAL_H
#define EVAL_H

#include "board.h"

/* Piece base values in centipawns */
#define VAL_PAWN    100
#define VAL_KNIGHT  320
#define VAL_BISHOP  330
#define VAL_ROOK    500
#define VAL_QUEEN   900
#define VAL_KING    20000

/*
 * Static evaluation function.
 * Returns score from the perspective of White (positive = White leads, negative = Black leads).
 */
int evaluate_board(const Board *board);

#endif /* EVAL_H */
