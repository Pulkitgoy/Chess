#include "../include/move.h"

void add_move(
    MoveList *list,
    Move move)
{
    list->moves[list->count] = move;
    list->count++;
}
