#include "../include/eval.h"
#include "../include/bitboard.h"
#include "../include/side.h"

/*
 * Piece-Square Tables (PST)
 * Represented from White's perspective (Rank 1 = sq 0..7, Rank 8 = sq 56..63).
 * For Black, square index is flipped vertically (sq ^ 56).
 */

static const int pawn_pst[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

static const int knight_pst[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const int bishop_pst[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int rook_pst[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

static const int queen_pst[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int king_pst[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

/* Helper function to score a set of piece bitboards */
static int score_pieces(U64 bb, int base_val, const int *pst, int is_white)
{
    int score = 0;
    while (bb)
    {
        int sq = get_lsb_index(bb);
        int pst_sq = is_white ? sq : (sq ^ 56);
        score += base_val + pst[pst_sq];
        clear_bit(&bb, sq);
    }
    return score;
}

int evaluate_board(const Board *board)
{
    int white_score = 0;
    int black_score = 0;

    /* White pieces */
    white_score += score_pieces(board->white_pawns,   VAL_PAWN,   pawn_pst,   1);
    white_score += score_pieces(board->white_knights, VAL_KNIGHT, knight_pst, 1);
    white_score += score_pieces(board->white_bishops, VAL_BISHOP, bishop_pst, 1);
    white_score += score_pieces(board->white_rooks,   VAL_ROOK,   rook_pst,   1);
    white_score += score_pieces(board->white_queens,  VAL_QUEEN,  queen_pst,  1);
    white_score += score_pieces(board->white_king,    VAL_KING,   king_pst,   1);

    /* Black pieces */
    black_score += score_pieces(board->black_pawns,   VAL_PAWN,   pawn_pst,   0);
    black_score += score_pieces(board->black_knights, VAL_KNIGHT, knight_pst, 0);
    black_score += score_pieces(board->black_bishops, VAL_BISHOP, bishop_pst, 0);
    black_score += score_pieces(board->black_rooks,   VAL_ROOK,   rook_pst,   0);
    black_score += score_pieces(board->black_queens,  VAL_QUEEN,  queen_pst,  0);
    black_score += score_pieces(board->black_king,    VAL_KING,   king_pst,   0);

    return white_score - black_score;
}
