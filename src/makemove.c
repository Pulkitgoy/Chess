#include "../include/makemove.h"
#include "../include/pieces.h"
#include "../include/bitboard.h"
#include "../include/board.h"
#include "../include/side.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/*  copy_board                                                          */
/* ------------------------------------------------------------------ */
void copy_board(Board *dst, const Board *src)
{
    memcpy(dst, src, sizeof(Board));
}

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*
 * get_piece_bb — return pointer to the bitboard for a given piece type.
 * This lets make_move handle any piece with a single switch instead of
 * 12 separate if-else chains.
 */
static U64 *get_piece_bb(Board *board, int piece)
{
    switch (piece)
    {
        case WHITE_PAWN:   return &board->white_pawns;
        case WHITE_KNIGHT: return &board->white_knights;
        case WHITE_BISHOP: return &board->white_bishops;
        case WHITE_ROOK:   return &board->white_rooks;
        case WHITE_QUEEN:  return &board->white_queens;
        case WHITE_KING:   return &board->white_king;
        case BLACK_PAWN:   return &board->black_pawns;
        case BLACK_KNIGHT: return &board->black_knights;
        case BLACK_BISHOP: return &board->black_bishops;
        case BLACK_ROOK:   return &board->black_rooks;
        case BLACK_QUEEN:  return &board->black_queens;
        case BLACK_KING:   return &board->black_king;
        default:           return NULL;
    }
}

/*
 * remove_piece — clear whichever piece occupies `sq` (for captures).
 * We never capture a king, so no king case needed.
 */
static void remove_piece(Board *board, int sq)
{
    if (get_bit(board->white_pawns,   sq)) { clear_bit(&board->white_pawns,   sq); return; }
    if (get_bit(board->white_knights, sq)) { clear_bit(&board->white_knights, sq); return; }
    if (get_bit(board->white_bishops, sq)) { clear_bit(&board->white_bishops, sq); return; }
    if (get_bit(board->white_rooks,   sq)) { clear_bit(&board->white_rooks,   sq); return; }
    if (get_bit(board->white_queens,  sq)) { clear_bit(&board->white_queens,  sq); return; }
    if (get_bit(board->black_pawns,   sq)) { clear_bit(&board->black_pawns,   sq); return; }
    if (get_bit(board->black_knights, sq)) { clear_bit(&board->black_knights, sq); return; }
    if (get_bit(board->black_bishops, sq)) { clear_bit(&board->black_bishops, sq); return; }
    if (get_bit(board->black_rooks,   sq)) { clear_bit(&board->black_rooks,   sq); return; }
    if (get_bit(board->black_queens,  sq)) { clear_bit(&board->black_queens,  sq); return; }
}

/* ------------------------------------------------------------------ */
/*  make_move                                                           */
/* ------------------------------------------------------------------ */
int make_move(Board *board, Move move)
{
    int source   = GET_SOURCE(move);
    int target   = GET_TARGET(move);
    int piece    = GET_PIECE(move);
    int promoted = GET_PROMOTED(move);
    int capture  = IS_CAPTURE(move);
    int dbl_push = IS_DOUBLE(move);
    int ep       = IS_EP(move);
    int castling = IS_CASTLING(move);

    int moving_side = board->side_to_move;

    /* --- Reset en-passant square (re-set below only on double push) --- */
    board->ep_square = -1;

    /* --- Lift the moving piece from its source square --- */
    U64 *piece_bb = get_piece_bb(board, piece);
    clear_bit(piece_bb, source);

    /* ---  Handle captures  --- */
    if (capture)
    {
        if (ep)
        {
            /*
             * En-passant: the captured pawn is NOT on `target` but one
             * rank behind it (relative to the mover).
             *   White captures upward  → captured pawn is at target - 8
             *   Black captures downward → captured pawn is at target + 8
             */
            int captured_sq = (moving_side == WHITE) ? target - 8 : target + 8;
            if (moving_side == WHITE)
                clear_bit(&board->black_pawns, captured_sq);
            else
                clear_bit(&board->white_pawns, captured_sq);
        }
        else
        {
            /*
             * Regular capture. Also revoke castling rights if a rook is
             * taken on its home square — the opponent can no longer castle
             * with a rook that no longer exists.
             */
            if (target == 0)  board->castling_rights &= ~CASTLE_WQ;
            if (target == 7)  board->castling_rights &= ~CASTLE_WK;
            if (target == 56) board->castling_rights &= ~CASTLE_BQ;
            if (target == 63) board->castling_rights &= ~CASTLE_BK;

            remove_piece(board, target);
        }
    }

    /* --- Place piece on target (promotion replaces pawn with new piece) --- */
    if (promoted)
    {
        U64 *promo_bb = get_piece_bb(board, promoted);
        set_bit(promo_bb, target);
        /* piece_bb still points to the pawn board; pawn already lifted above */
    }
    else
    {
        set_bit(piece_bb, target);
    }

    /* --- Double pawn push: record the skipped square as en-passant target --- */
    if (dbl_push)
    {
        board->ep_square = (moving_side == WHITE) ? target - 8 : target + 8;
    }

    /* --- Castling: also move the rook --- */
    if (castling)
    {
        switch (target)
        {
            case 6:  /* White O-O:  rook h1 → f1 */
                clear_bit(&board->white_rooks, 7);
                set_bit(&board->white_rooks, 5);
                break;
            case 2:  /* White O-O-O: rook a1 → d1 */
                clear_bit(&board->white_rooks, 0);
                set_bit(&board->white_rooks, 3);
                break;
            case 62: /* Black O-O:  rook h8 → f8 */
                clear_bit(&board->black_rooks, 63);
                set_bit(&board->black_rooks, 61);
                break;
            case 58: /* Black O-O-O: rook a8 → d8 */
                clear_bit(&board->black_rooks, 56);
                set_bit(&board->black_rooks, 59);
                break;
        }
    }

    /*
     * --- Update castling rights based on king/rook moves ---
     * If the king moves, both castling rights for that side are revoked.
     * If a rook moves from its home square, that side's right is revoked.
     */
    switch (piece)
    {
        case WHITE_KING:
            board->castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
            break;
        case BLACK_KING:
            board->castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
            break;
        case WHITE_ROOK:
            if (source == 0) board->castling_rights &= ~CASTLE_WQ;
            if (source == 7) board->castling_rights &= ~CASTLE_WK;
            break;
        case BLACK_ROOK:
            if (source == 56) board->castling_rights &= ~CASTLE_BQ;
            if (source == 63) board->castling_rights &= ~CASTLE_BK;
            break;
        default:
            break;
    }

    update_occupancies(board);
    board->side_to_move ^= 1;  /* flip side */
    return 1;
}
