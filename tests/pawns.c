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

void pawn_test()
{
    Board board;

    // init_start_position(&board);
    memset(&board, 0, sizeof(Board));
    set_bit(&board.white_pawns, 8);    // a2
    set_bit(&board.white_pawns, 24);    // a4
    set_bit(&board.black_pawns, 17);    // b3
    set_bit(&board.black_pawns, 9);    // b2
    // set_bit(&board.black_rooks, 57);   // b8
    update_occupancies(&board);

    board.side_to_move = WHITE;
    init_leaper_attacks();
    
    MoveList list = {0};
    generate_pawn_moves(
        &board,
        &list,
        WHITE
    );
    generate_pawn_moves(
        &board,
        &list,
        BLACK
    );
    // generate_knight_moves(
    //     &board,
    //     &list,
    //     WHITE
    // );
    printf("WHITE has %d moves\n", list.count);

    for (int i = 0; i < list.count; i++)
    {
        print_move(list.moves[i]);
        switch(GET_PROMOTED(list.moves[i]))
        {
            case WHITE_QUEEN:
                printf("=Q\n");
                break;
            case WHITE_ROOK:
                printf("=R\n");
                break;
            case WHITE_BISHOP:
                printf("=B\n");
                break;
            case WHITE_KNIGHT:
                printf("=N\n");
                break;
            case BLACK_QUEEN:
                printf("=q\n");
                break;
            case BLACK_ROOK:
                printf("=r\n");
                break;
            case BLACK_BISHOP:
                printf("=b\n");
                break;
            case BLACK_KNIGHT:
                printf("=n\n");
                break;
            default:
                printf("\n");
                break;
        }
    }
}