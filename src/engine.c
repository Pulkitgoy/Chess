#include "../include/engine.h"
#include "../include/eval.h"
#include "../include/movegen.h"
#include "../include/makemove.h"
#include "../include/pieces.h"
#include "../include/side.h"
#include "../include/bitboard.h"
#include <stdlib.h>

/* Victim and attacker piece weights for MVV-LVA move ordering */
static const int piece_weights[12] = {
    [WHITE_PAWN]   = 1, [WHITE_KNIGHT] = 2, [WHITE_BISHOP] = 3,
    [WHITE_ROOK]   = 4, [WHITE_QUEEN]  = 5, [WHITE_KING]   = 6,
    [BLACK_PAWN]   = 1, [BLACK_KNIGHT] = 2, [BLACK_BISHOP] = 3,
    [BLACK_ROOK]   = 4, [BLACK_QUEEN]  = 5, [BLACK_KING]   = 6
};

/* Evaluate move priority for MVV-LVA ordering */
static int score_move(const Board *board, Move move)
{
    int score = 0;

    if (IS_CAPTURE(move))
    {
        int src_piece = GET_PIECE(move);
        int target_sq = GET_TARGET(move);
        int target_piece = WHITE_PAWN; /* default fallback for EP */

        /* Find piece type on target square */
        if (!IS_EP(move))
        {
            if (get_bit(board->white_pawns,   target_sq)) target_piece = WHITE_PAWN;
            else if (get_bit(board->white_knights, target_sq)) target_piece = WHITE_KNIGHT;
            else if (get_bit(board->white_bishops, target_sq)) target_piece = WHITE_BISHOP;
            else if (get_bit(board->white_rooks,   target_sq)) target_piece = WHITE_ROOK;
            else if (get_bit(board->white_queens,  target_sq)) target_piece = WHITE_QUEEN;
            else if (get_bit(board->black_pawns,   target_sq)) target_piece = BLACK_PAWN;
            else if (get_bit(board->black_knights, target_sq)) target_piece = BLACK_KNIGHT;
            else if (get_bit(board->black_bishops, target_sq)) target_piece = BLACK_BISHOP;
            else if (get_bit(board->black_rooks,   target_sq)) target_piece = BLACK_ROOK;
            else if (get_bit(board->black_queens,  target_sq)) target_piece = BLACK_QUEEN;
        }

        int victim_val   = piece_weights[target_piece];
        int attacker_val = piece_weights[src_piece];

        /* MVV-LVA formula: 10 * victim - attacker */
        score += 1000 + (10 * victim_val - attacker_val);
    }

    if (GET_PROMOTED(move))
    {
        score += 800;
    }

    return score;
}

/* Sort moves in place based on heuristic scores */
static void sort_moves(const Board *board, MoveList *list)
{
    int scores[256];
    for (int i = 0; i < list->count; i++)
    {
        scores[i] = score_move(board, list->moves[i]);
    }

    for (int i = 0; i < list->count - 1; i++)
    {
        for (int j = i + 1; j < list->count; j++)
        {
            if (scores[j] > scores[i])
            {
                int temp_score = scores[i];
                scores[i] = scores[j];
                scores[j] = temp_score;

                Move temp_move = list->moves[i];
                list->moves[i] = list->moves[j];
                list->moves[j] = temp_move;
            }
        }
    }
}

/*
 * Negamax algorithm with Alpha-Beta Pruning.
 * Returns evaluation relative to side_to_move at current node.
 */
static int negamax(Board *board, int depth, int ply, int alpha, int beta, Move *best_move_out, U64 *nodes_count)
{
    (*nodes_count)++;

    if (depth == 0)
    {
        int static_eval = evaluate_board(board);
        return (board->side_to_move == WHITE) ? static_eval : -static_eval;
    }

    MoveList list = {0};
    generate_moves(board, &list);

    /* Terminal node check: Checkmate or Stalemate */
    if (list.count == 0)
    {
        U64 king_bb = (board->side_to_move == WHITE) ? board->white_king : board->black_king;
        int king_sq = king_bb ? get_lsb_index(king_bb) : -1;

        if (king_sq >= 0 && is_square_attacked(board, king_sq, 1 - board->side_to_move))
        {
            /* Checkmate: penalize by ply to favor shorter checkmates */
            return -(MATE_SCORE - ply);
        }
        else
        {
            /* Stalemate: neutral score */
            return 0;
        }
    }

    sort_moves(board, &list);

    Move best_move = list.moves[0];

    for (int i = 0; i < list.count; i++)
    {
        Board copy;
        copy_board(&copy, board);
        make_move(&copy, list.moves[i]);

        int score = -negamax(&copy, depth - 1, ply + 1, -beta, -alpha, NULL, nodes_count);

        if (score > alpha)
        {
            alpha = score;
            best_move = list.moves[i];

            if (alpha >= beta)
            {
                /* Alpha-Beta Cutoff */
                break;
            }
        }
    }

    if (best_move_out)
    {
        *best_move_out = best_move;
    }

    return alpha;
}

Move search_best_move(Board *board, int depth, int *score_out, U64 *nodes_out)
{
    U64 nodes_count = 0;
    Move best_move = 0;

    int eval = negamax(board, depth, 0, -INF_SCORE, INF_SCORE, &best_move, &nodes_count);

    if (score_out) *score_out = eval;
    if (nodes_out) *nodes_out = nodes_count;

    return best_move;
}
