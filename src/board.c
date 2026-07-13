#include "../include/board.h"
#include "../include/side.h"
#include <stdio.h>

void update_occupancies(Board *board)
{
    board->white_occ =
          board->white_pawns
        | board->white_knights
        | board->white_bishops
        | board->white_rooks
        | board->white_queens
        | board->white_king;

    board->black_occ =
          board->black_pawns
        | board->black_knights
        | board->black_bishops
        | board->black_rooks
        | board->black_queens
        | board->black_king;

    board->all_occ =
          board->white_occ
        | board->black_occ;

}

void init_start_position(Board *board)
{
    board->white_pawns   = 0x000000000000FF00ULL;
    board->white_rooks   = 0x0000000000000081ULL;
    board->white_knights = 0x0000000000000042ULL;
    board->white_bishops = 0x0000000000000024ULL;
    board->white_queens  = 0x0000000000000008ULL;
    board->white_king    = 0x0000000000000010ULL;

    board->black_pawns   = 0x00FF000000000000ULL;
    board->black_rooks   = 0x8100000000000000ULL;
    board->black_knights = 0x4200000000000000ULL;
    board->black_bishops = 0x2400000000000000ULL;
    board->black_queens  = 0x0800000000000000ULL;
    board->black_king    = 0x1000000000000000ULL;

    board->side_to_move = WHITE;

    update_occupancies(board);
}


void print_board(Board *board)
{
    for(int rank = 7; rank >= 0; rank--)
    {
        printf("%d  ", rank + 1);

        for(int file = 0; file < 8; file++)
        {
            int sq = rank * 8 + file;

            char piece = '.';

            if(get_bit(board->white_pawns, sq)) piece = 'P';
            else if(get_bit(board->white_knights, sq)) piece = 'N';
            else if(get_bit(board->white_bishops, sq)) piece = 'B';
            else if(get_bit(board->white_rooks, sq)) piece = 'R';
            else if(get_bit(board->white_queens, sq)) piece = 'Q';
            else if(get_bit(board->white_king, sq)) piece = 'K';

            else if(get_bit(board->black_pawns, sq)) piece = 'p';
            else if(get_bit(board->black_knights, sq)) piece = 'n';
            else if(get_bit(board->black_bishops, sq)) piece = 'b';
            else if(get_bit(board->black_rooks, sq)) piece = 'r';
            else if(get_bit(board->black_queens, sq)) piece = 'q';
            else if(get_bit(board->black_king, sq)) piece = 'k';

            printf("%c ", piece);
        }

        printf("\n");
    }

    printf("\n   a b c d e f g h\n\n");

    printf("Side to move: %s\n",
           board->side_to_move == WHITE ? "White" : "Black");
}