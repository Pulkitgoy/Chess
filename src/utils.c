#include "../include/utils.h"
#include <stdio.h>

void print_move(Move move)
{
    int source = GET_SOURCE(move);
    int target = GET_TARGET(move);

    char files[] = "abcdefgh";

    printf(
        "%c%d%c%d\n",
        files[source % 8],
        source / 8 + 1,
        files[target % 8],
        target / 8 + 1
    );
}