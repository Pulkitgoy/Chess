#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "board.h"
#include "move.h"

/* Apply a move to the board (modifies board in place). Returns 1. */
int  make_move(Board *board, Move move);

/* Copy board state (for legality checking). */
void copy_board(Board *dst, const Board *src);

#endif