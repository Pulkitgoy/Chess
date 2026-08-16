#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"

/*
 * is_square_attacked:
 *   Returns 1 if square `sq` is attacked by any piece of color `by_side`.
 *   Used for check detection and castling legality.
 */
int is_square_attacked(Board *board, int sq, int by_side);

/*
 * generate_moves:
 *   Generate ALL legal moves for board->side_to_move.
 *   Filters out pseudo-legal moves that leave the own king in check.
 */
void generate_moves(Board *board, MoveList *list);

/* Per-piece pseudo-legal generators */
void generate_pawn_moves  (Board *board, MoveList *list, int side);
void generate_knight_moves(Board *board, MoveList *list, int side);
void generate_king_moves  (Board *board, MoveList *list, int side);

#endif