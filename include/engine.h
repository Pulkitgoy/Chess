#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "move.h"

#define INF_SCORE  1000000
#define MATE_SCORE 100000

/*
 * search_best_move:
 * Executes a Minimax search with Alpha-Beta pruning to find the optimal move.
 *
 * Parameters:
 *   board     - pointer to current board position
 *   depth     - search depth in plies (e.g. 4)
 *   score_out - optional pointer to store evaluation score (from perspective of side to move)
 *   nodes_out - optional pointer to store total node count
 *
 * Returns: Best Move integer (or 0 if no legal move exists).
 */
Move search_best_move(Board *board, int depth, int *score_out, U64 *nodes_out);

#endif /* ENGINE_H */
