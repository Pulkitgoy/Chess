#include "../include/board.h"

int main(void)
{
    Board board;

    init_start_position(&board);

    print_board(&board);

    return 0;
}