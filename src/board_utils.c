#include "../include/board_utils.h"
#include "../include/bitboard.h"
#include "../include/pieces.h"

int get_piece_on_square(
    Board *board,
    int square
)
{
    if(get_bit(board->white_pawns, square))
        return WHITE_PAWN;

    if(get_bit(board->white_knights, square))
        return WHITE_KNIGHT;

    if(get_bit(board->white_bishops, square))
        return WHITE_BISHOP;

    if(get_bit(board->white_rooks, square))
        return WHITE_ROOK;

    if(get_bit(board->white_queens, square))
        return WHITE_QUEEN;

    if(get_bit(board->white_king, square))
        return WHITE_KING;

    if(get_bit(board->black_pawns, square))
        return BLACK_PAWN;

    if(get_bit(board->black_knights, square))
        return BLACK_KNIGHT;

    if(get_bit(board->black_bishops, square))
        return BLACK_BISHOP;

    if(get_bit(board->black_rooks, square))
        return BLACK_ROOK;

    if(get_bit(board->black_queens, square))
        return BLACK_QUEEN;

    if(get_bit(board->black_king, square))
        return BLACK_KING;

    return -1;
}