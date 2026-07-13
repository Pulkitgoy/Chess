#include "../include/board.h"
#include "../include/bitboard.h"
#include "../include/attacks.h"
#include "../include/move.h"
#include "../include/pieces.h"
#include "../include/movegen.h"
#include "../include/utils.h"
#include "../include/board_utils.h"
#include "../include/makemove.h"
#include "../include/side.h"
#include "../include/sliding.h"
#include <stdio.h>
#include <string.h>
int main()
{
    Board board;

    init_start_position(&board);
    // memset(&board, 0, sizeof(Board));
    init_leaper_attacks();

    Move move =
        ENCODE_MOVE(
            12,
            28,
            WHITE_PAWN,
            0,
            0,
            1,
            0,
            0
        );

    make_move(&board, move);
    board.white_bishops = 0;
    board.black_bishops = 0;

    set_bit(&board.white_bishops, 27);   // d4

    update_occupancies(&board);
    MoveList list = {0};
    generate_bishop_moves(
        &board,
        &list,
        WHITE
    );

    // generate_black_knight_moves(
    //     &board,
    //     &list
    // );

    printf("Black has %d moves\n", list.count);

    for (int i = 0; i < list.count; i++)
    {
        print_move(list.moves[i]);
    }

    return 0;
}