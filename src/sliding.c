#include "../include/sliding.h"
#include "../include/bitboard.h"
#include "../include/pieces.h"
#include "../include/board_utils.h"
#include "../include/side.h"
#include<stdio.h>
static const int bishopDirections[4][2] =
{
    { 1,  1},   // NE
    { 1, -1},   // NW
    {-1,  1},   // SE
    {-1, -1}    // SW
};

static const int rookDirections[4][2] =
{
    { 1,  0},   // N
    {-1,  0},   // S
    { 0,  1},   // E
    { 0, -1}    // W
};

static const int queenDirections[8][2] =
{
    { 1,  0},
    {-1,  0},
    { 0,  1},
    { 0, -1},
    { 1,  1},
    { 1, -1},
    {-1,  1},
    {-1, -1}
};

void generate_sliding_moves(
    Board *board,
    MoveList *list,
    int side,
    const int directions[][2],
    int directionCount,
    U64 pieces,
    int piece
)
{
    U64 own_occ;
    U64 enemy_occ;

    U64 sliders = pieces;
    if(side == WHITE)
    {
        // sliders = board->white_bishops;
        own_occ = board->white_occ;
        enemy_occ = board->black_occ;
    }
    else
    {
        // sliders = board->black_bishops;
        own_occ = board->black_occ;
        enemy_occ = board->white_occ;
    }

    while(sliders)
    {
        int source = get_lsb_index(sliders);
        int sourceRank = source / 8;
        int sourceFile = source % 8;
        for(int dir=0 ; dir<directionCount ; dir++)
        {
            int rank = sourceRank;
            int file = sourceFile;
            while(1)
            {
                rank+=directions[dir][0];
                file+=directions[dir][1];
                if(rank<0 || rank>7 || file<0 || file>7)
                    break;
                int target = rank * 8 + file;
                if (own_occ & (1ULL << target))
                    break;
                add_move(
                    list,
                    ENCODE_MOVE(
                        source,
                        target,
                        piece,
                        0,
                        (enemy_occ & (1ULL << target)) ? 1 : 0,
                        0,
                        0,
                        0
                    )
                );
                if (enemy_occ & (1ULL << target))
                    break; 
            }
        }
        clear_bit(&sliders, source);
    }
}


void generate_bishop_moves(
    Board *board,
    MoveList *list,
    int side
)
{
    U64 bishops =
        (side == WHITE)
        ? board->white_bishops
        : board->black_bishops;

    generate_sliding_moves(
        board,
        list,
        side,
        bishopDirections,
        4,
        bishops,
        side == WHITE
            ? WHITE_BISHOP
            : BLACK_BISHOP
    );
}

void generate_rook_moves(
    Board *board,
    MoveList *list,
    int side
)
{
    U64 rooks =
        (side == WHITE)
        ? board->white_rooks
        : board->black_rooks;

    generate_sliding_moves(
        board,
        list,
        side,
        rookDirections,
        4,
        rooks,
        side == WHITE
            ? WHITE_ROOK
            : BLACK_ROOK
    );
}

void generate_queen_moves(
    Board *board,
    MoveList *list,
    int side
)
{
    U64 queens =
        (side == WHITE)
        ? board->white_queens
        : board->black_queens;

    generate_sliding_moves(
        board,
        list,
        side,
        queenDirections,
        8,
        queens,
        side == WHITE
            ? WHITE_QUEEN
            : BLACK_QUEEN
    );
}