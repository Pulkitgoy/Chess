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

void generate_bishop_moves(
    Board *board,
    MoveList *list,
    int side
)
{
    U64 bishops;
    U64 own_occ;
    U64 enemy_occ;

    int piece;
    if(side == WHITE)
    {
        bishops = board->white_bishops;
        own_occ = board->white_occ;
        enemy_occ = board->black_occ;

        piece = WHITE_BISHOP;
    }
    else
    {
        bishops = board->black_bishops;
        own_occ = board->black_occ;
        enemy_occ = board->white_occ;

        piece = BLACK_BISHOP;
    }

    while(bishops)
    {
        int source = get_lsb_index(bishops);
        int sourceRank = source / 8;
        int sourceFile = source % 8;
        for(int dir=0 ; dir<4 ; dir++)
        {
            int rank = sourceRank;
            int file = sourceFile;
            while(1)
            {
                rank+=bishopDirections[dir][0];
                file+=bishopDirections[dir][1];
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
                        enemy_occ & (1ULL << target),
                        0,
                        0,
                        0
                    )
                );
                if (enemy_occ & (1ULL << target))
                    break; 
            }
        }
        clear_bit(&bishops, source);
    }
}

// void generate_rook_moves(
//     Board *board,
//     MoveList *list,
//     int side
// )
// {
//     U64 rooks;
//     U64 own_occ;
//     U64 enemy_occ;

//     int piece;
//     if(side == WHITE)
//     {
//         rooks = board->white_rooks;
//         own_occ = board->white_occ;
//         enemy_occ = board->black_occ;

//         piece = WHITE_ROOK;
//     }
//     else
//     {
//         rooks = board->black_rooks;
//         own_occ = board->black_occ;
//         enemy_occ = board->white_occ;

//         piece = BLACK_ROOK;
//     }

//     while(rooks)
//     {
//         int source = get_lsb_index(rooks);
//         int sourceRank = source / 8;
//         int sourceFile = source % 8;
//         for(int dir=0 ; dir<4 ; dir++)
//         {
//             int rank = sourceRank;
//             int file = sourceFile;
//             while(1)
//             {
//                 rank+=rookDirections[dir][0];
//                 file+=rookDirections[dir][1];
//                 if(rank<0 || rank>7 || file<0 || file>7)
//                     break;
//                 int target = rank * 8 + file;
//                 if (own_occ & (1ULL << target))
//                     break;
//                 add_move(
//                     list,
//                     ENCODE_MOVE(
//                         source,
//                         target,
//                         piece,
//                         0,
//                         enemy_occ & (1ULL << target),
//                         0,
//                         0,
//                         0
//                     )
//                 );
//                 if (enemy_occ & (1ULL << target))
//                     break; 
//             }
//         }
//         clear_bit(&rooks, source);
//     }
// }

// void generate_sliding_moves(
//     Board *board,
//     MoveList *list,
//     int side,
//     const int directions[][2],
//     int directionCount,
//     U64 pieces,
//     int piece
// )
// {
//     ...
// }