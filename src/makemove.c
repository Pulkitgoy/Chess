#include "../include/makemove.h"
#include "../include/pieces.h"
#include "../include/board_utils.h"
#include "../include/bitboard.h"

int make_move(
    Board *board,
    Move move
)
{
    int source =
        GET_SOURCE(move);

    int target =
        GET_TARGET(move);

    int piece =
        get_piece_on_square(
            board,
            source
        );

    switch(piece)
    {
        case WHITE_PAWN:

            clear_bit(
                &board->white_pawns,
                source
            );

            set_bit(
                &board->white_pawns,
                target
            );

            break;

        case WHITE_KNIGHT:

            clear_bit(
                &board->white_knights,
                source
            );

            set_bit(
                &board->white_knights,
                target
            );

            break;
        case BLACK_PAWN:

            clear_bit(
                &board->black_pawns,
                source
            );

            set_bit(
                &board->black_pawns,
                target
            );

            break;

        case BLACK_KNIGHT:

            clear_bit(
                &board->black_knights,
                source
            );

            set_bit(
                &board->black_knights,
                target
            );

            break;
    }

    update_occupancies(board);

    board->side_to_move ^= 1;

    return 1;
}
