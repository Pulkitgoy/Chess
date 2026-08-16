#include "../include/movegen.h"
#include "../include/makemove.h"
#include "../include/sliding.h"
#include "../include/pieces.h"
#include "../include/attacks.h"
#include "../include/bitboard.h"
#include "../include/side.h"
#include "../include/utils.h"

/* ================================================================== */
/*  is_square_attacked                                                  */
/* ================================================================== */
/*
 * Returns 1 if `sq` is attacked by any piece of color `by_side`.
 *
 * Key trick for pawns:
 *   A white pawn on X attacks sq  ⟺  sq is in pawn_attacks[WHITE][X].
 *   Equivalently: X is in pawn_attacks[BLACK][sq]  (the "reverse" view).
 *   So we use pawn_attacks[opposite][sq] & <attacker_pawns>.
 */
int is_square_attacked(Board *board, int sq, int by_side)
{
    /* ---- Pawn ---- */
    if (by_side == WHITE)
    {
        if (pawn_attacks[BLACK][sq] & board->white_pawns)   return 1;
        if (knight_attacks[sq]      & board->white_knights) return 1;
        if (king_attacks[sq]        & board->white_king)    return 1;
    }
    else
    {
        if (pawn_attacks[WHITE][sq] & board->black_pawns)   return 1;
        if (knight_attacks[sq]      & board->black_knights) return 1;
        if (king_attacks[sq]        & board->black_king)    return 1;
    }

    /* ---- Bishop / Queen (diagonal rays) ---- */
    U64 diag_attackers = (by_side == WHITE)
        ? (board->white_bishops | board->white_queens)
        : (board->black_bishops | board->black_queens);

    static const int diag_dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    int sq_rank = sq / 8;
    int sq_file = sq % 8;

    for (int d = 0; d < 4; d++)
    {
        int r = sq_rank, f = sq_file;
        while (1)
        {
            r += diag_dirs[d][0];
            f += diag_dirs[d][1];
            if (r < 0 || r > 7 || f < 0 || f > 7) break;
            int s = r * 8 + f;
            if (board->all_occ & (1ULL << s))
            {
                if (diag_attackers & (1ULL << s)) return 1;
                break; /* blocked by a non-attacker piece */
            }
        }
    }

    /* ---- Rook / Queen (straight rays) ---- */
    U64 straight_attackers = (by_side == WHITE)
        ? (board->white_rooks | board->white_queens)
        : (board->black_rooks | board->black_queens);

    static const int straight_dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    for (int d = 0; d < 4; d++)
    {
        int r = sq_rank, f = sq_file;
        while (1)
        {
            r += straight_dirs[d][0];
            f += straight_dirs[d][1];
            if (r < 0 || r > 7 || f < 0 || f > 7) break;
            int s = r * 8 + f;
            if (board->all_occ & (1ULL << s))
            {
                if (straight_attackers & (1ULL << s)) return 1;
                break;
            }
        }
    }

    return 0;
}

/* ================================================================== */
/*  Pawn move generation                                               */
/* ================================================================== */
void generate_pawn_moves(Board *board, MoveList *list, int side)
{
    U64 pawns;
    U64 enemy_occ;
    int direction;
    int start_rank_min, start_rank_max;
    int piece;
    int promotion_rank;
    int Queen, Rook, Bishop, Knight;

    if (side == WHITE)
    {
        promotion_rank  = 7;
        pawns           = board->white_pawns;
        enemy_occ       = board->black_occ;
        direction       = 8;
        start_rank_min  = 8;
        start_rank_max  = 15;
        piece           = WHITE_PAWN;
        Queen = WHITE_QUEEN; Rook = WHITE_ROOK;
        Bishop = WHITE_BISHOP; Knight = WHITE_KNIGHT;
    }
    else
    {
        promotion_rank  = 0;
        pawns           = board->black_pawns;
        enemy_occ       = board->white_occ;
        direction       = -8;
        start_rank_min  = 48;
        start_rank_max  = 55;
        piece           = BLACK_PAWN;
        Queen = BLACK_QUEEN; Rook = BLACK_ROOK;
        Bishop = BLACK_BISHOP; Knight = BLACK_KNIGHT;
    }

    while (pawns)
    {
        int source = get_lsb_index(pawns);
        int target = source + direction;

        /* ---- Single push ---- */
        if (!(board->all_occ & (1ULL << target)))
        {
            if (target / 8 == promotion_rank)
            {
                add_move(list, ENCODE_MOVE(source, target, piece, Queen,  0,0,0,0));
                add_move(list, ENCODE_MOVE(source, target, piece, Rook,   0,0,0,0));
                add_move(list, ENCODE_MOVE(source, target, piece, Bishop, 0,0,0,0));
                add_move(list, ENCODE_MOVE(source, target, piece, Knight, 0,0,0,0));
            }
            else
            {
                add_move(list, ENCODE_MOVE(source, target, piece, 0, 0,0,0,0));
            }

            /* ---- Double push (only when square in front is also clear) ---- */
            if (source >= start_rank_min && source <= start_rank_max)
            {
                int dbl_target = source + 2 * direction;
                if (!(board->all_occ & (1ULL << dbl_target)))
                    add_move(list, ENCODE_MOVE(source, dbl_target, piece, 0, 0,1,0,0));
            }
        }

        /* ---- Diagonal captures ---- */
        U64 attacks = (side == WHITE)
            ? pawn_attacks[WHITE][source]
            : pawn_attacks[BLACK][source];
        attacks &= enemy_occ;

        while (attacks)
        {
            int cap_target = get_lsb_index(attacks);
            if (cap_target / 8 == promotion_rank)
            {
                add_move(list, ENCODE_MOVE(source, cap_target, piece, Queen,  1,0,0,0));
                add_move(list, ENCODE_MOVE(source, cap_target, piece, Rook,   1,0,0,0));
                add_move(list, ENCODE_MOVE(source, cap_target, piece, Bishop, 1,0,0,0));
                add_move(list, ENCODE_MOVE(source, cap_target, piece, Knight, 1,0,0,0));
            }
            else
            {
                add_move(list, ENCODE_MOVE(source, cap_target, piece, 0, 1,0,0,0));
            }
            clear_bit(&attacks, cap_target);
        }

        /* ---- En-passant capture ---- */
        if (board->ep_square != -1)
        {
            U64 ep_atk = (side == WHITE)
                ? pawn_attacks[WHITE][source]
                : pawn_attacks[BLACK][source];
            if (ep_atk & (1ULL << board->ep_square))
                add_move(list, ENCODE_MOVE(source, board->ep_square, piece, 0, 1,0,1,0));
        }

        clear_bit(&pawns, source);
    }
}

/* ================================================================== */
/*  Knight move generation                                             */
/* ================================================================== */
void generate_knight_moves(Board *board, MoveList *list, int side)
{
    U64 knights, own_occ, enemy_occ;
    int piece;

    if (side == WHITE)
    {
        knights   = board->white_knights;
        own_occ   = board->white_occ;
        enemy_occ = board->black_occ;
        piece     = WHITE_KNIGHT;
    }
    else
    {
        knights   = board->black_knights;
        own_occ   = board->black_occ;
        enemy_occ = board->white_occ;
        piece     = BLACK_KNIGHT;
    }

    while (knights)
    {
        int source  = get_lsb_index(knights);
        U64 attacks = knight_attacks[source] & ~own_occ;
        while (attacks)
        {
            int target  = get_lsb_index(attacks);
            int capture = (enemy_occ & (1ULL << target)) ? 1 : 0;
            add_move(list, ENCODE_MOVE(source, target, piece, 0, capture, 0,0,0));
            clear_bit(&attacks, target);
        }
        clear_bit(&knights, source);
    }
}

/* ================================================================== */
/*  King move generation (regular + castling)                         */
/* ================================================================== */
void generate_king_moves(Board *board, MoveList *list, int side)
{
    U64 kings, own_occ, enemy_occ;
    int piece;

    if (side == WHITE)
    {
        kings     = board->white_king;
        own_occ   = board->white_occ;
        enemy_occ = board->black_occ;
        piece     = WHITE_KING;
    }
    else
    {
        kings     = board->black_king;
        own_occ   = board->black_occ;
        enemy_occ = board->white_occ;
        piece     = BLACK_KING;
    }

    while (kings)
    {
        int source  = get_lsb_index(kings);
        U64 attacks = king_attacks[source] & ~own_occ;
        while (attacks)
        {
            int target  = get_lsb_index(attacks);
            int capture = (enemy_occ & (1ULL << target)) ? 1 : 0;
            add_move(list, ENCODE_MOVE(source, target, piece, 0, capture, 0,0,0));
            clear_bit(&attacks, target);
        }
        clear_bit(&kings, source);
    }

    /*
     * ---- Castling ----
     * Rules (full legality per FIDE):
     *   1. Correct castling right must be set.
     *   2. All squares between king and rook must be empty.
     *   3. King must NOT currently be in check.
     *   4. King must NOT pass through an attacked square.
     *   5. King must NOT land on an attacked square.
     *      (conditions 3-5 are checked via is_square_attacked)
     *
     * The legality filter in generate_moves will also discard any move
     * that leaves the king in check, providing a second safety net.
     */

    int opp = 1 - side;

    if (side == WHITE)
    {
        /* White kingside  (e1=4 → g1=6, rook f1=5, g1=6 must be empty) */
        if ((board->castling_rights & CASTLE_WK) &&
            !(board->all_occ & (1ULL << 5)) &&
            !(board->all_occ & (1ULL << 6)) &&
            !is_square_attacked(board, 4, opp) &&
            !is_square_attacked(board, 5, opp) &&
            !is_square_attacked(board, 6, opp))
        {
            add_move(list, ENCODE_MOVE(4, 6, WHITE_KING, 0, 0,0,0,1));
        }

        /* White queenside (e1=4 → c1=2, squares b1=1,c1=2,d1=3 empty,
           king passes through d1=3,c1=2 — both checked for attacks)      */
        if ((board->castling_rights & CASTLE_WQ) &&
            !(board->all_occ & (1ULL << 1)) &&
            !(board->all_occ & (1ULL << 2)) &&
            !(board->all_occ & (1ULL << 3)) &&
            !is_square_attacked(board, 4, opp) &&
            !is_square_attacked(board, 3, opp) &&
            !is_square_attacked(board, 2, opp))
        {
            add_move(list, ENCODE_MOVE(4, 2, WHITE_KING, 0, 0,0,0,1));
        }
    }
    else
    {
        /* Black kingside  (e8=60 → g8=62, f8=61, g8=62 empty) */
        if ((board->castling_rights & CASTLE_BK) &&
            !(board->all_occ & (1ULL << 61)) &&
            !(board->all_occ & (1ULL << 62)) &&
            !is_square_attacked(board, 60, opp) &&
            !is_square_attacked(board, 61, opp) &&
            !is_square_attacked(board, 62, opp))
        {
            add_move(list, ENCODE_MOVE(60, 62, BLACK_KING, 0, 0,0,0,1));
        }

        /* Black queenside (e8=60 → c8=58, b8=57,c8=58,d8=59 empty) */
        if ((board->castling_rights & CASTLE_BQ) &&
            !(board->all_occ & (1ULL << 57)) &&
            !(board->all_occ & (1ULL << 58)) &&
            !(board->all_occ & (1ULL << 59)) &&
            !is_square_attacked(board, 60, opp) &&
            !is_square_attacked(board, 59, opp) &&
            !is_square_attacked(board, 58, opp))
        {
            add_move(list, ENCODE_MOVE(60, 58, BLACK_KING, 0, 0,0,0,1));
        }
    }
}

/* ================================================================== */
/*  generate_moves — master legal move generator                      */
/* ================================================================== */
/*
 * Strategy:
 *   1. Generate all pseudo-legal moves (may leave king in check).
 *   2. For each, copy the board, apply the move, then check whether
 *      the moving side's king is under attack.  If yes, discard.
 *
 * copy_board (memcpy) is the deep-C struct-copy technique used here to
 * create an independent snapshot of full board state without dynamic
 * allocation.
 */
void generate_moves(Board *board, MoveList *list)
{
    MoveList pseudo = {0};
    int side = board->side_to_move;

    generate_pawn_moves  (board, &pseudo, side);
    generate_knight_moves(board, &pseudo, side);
    generate_bishop_moves(board, &pseudo, side);
    generate_rook_moves  (board, &pseudo, side);
    generate_queen_moves (board, &pseudo, side);
    generate_king_moves  (board, &pseudo, side);

    for (int i = 0; i < pseudo.count; i++)
    {
        Board copy;
        copy_board(&copy, board);
        make_move(&copy, pseudo.moves[i]);

        /* After the move, it's the opponent's turn; find OUR king. */
        U64 king_bb = (side == WHITE) ? copy.white_king : copy.black_king;
        if (!king_bb) continue; /* safety guard */
        int king_sq = get_lsb_index(king_bb);

        /* Keep the move only if our king is NOT attacked after it */
        if (!is_square_attacked(&copy, king_sq, 1 - side))
            add_move(list, pseudo.moves[i]);
    }
}