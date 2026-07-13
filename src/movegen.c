#include "../include/movegen.h"
#include "../include/pieces.h"
#include "../include/attacks.h"
#include "../include/bitboard.h"
#include "../include/pieces.h"
#include "../include/side.h"
// #include "../include/movegen.h"
// #include "../include/utils.h"

void generate_pawn_moves(
    Board *board,
    MoveList *list,
    int side)
{
    U64 pawns;
    int direction;
    int start_rank_min;
    int start_rank_max;
    int piece;

    if(side == WHITE)
    {
        pawns = board->white_pawns;
        direction = 8;
        start_rank_min = 8;
        start_rank_max = 15;
        piece = WHITE_PAWN;
    }
    else
    {
        pawns = board->black_pawns;
        direction = -8;
        start_rank_min = 48;
        start_rank_max = 55;
        piece = BLACK_PAWN;
    }

    while(pawns)
    {
        int source =
        get_lsb_index(pawns);
        
        int target =
        source + direction;
        
        if(!(board->all_occ & (1ULL << target)))
        {
            add_move(
                list,
                ENCODE_MOVE(
                    source,
                    target,
                    piece,
                    0,
                    0,
                    0,
                    0,
                    0
                )
            );
        }
        if(source >= start_rank_min && source <= start_rank_max)
        {
            if(!(board->all_occ & (1ULL << (source + direction))) &&
            !(board->all_occ & (1ULL << (source + 2 * direction))))
            {
                add_move(
                    list,
                    ENCODE_MOVE(
                        source,
                        source + 2 * direction,
                        piece,
                        0,
                        0,
                        1,
                        0,
                        0
                    )
                );
            }
        }
        clear_bit(&pawns, source);
    }
}


void generate_knight_moves(
    Board *board,
    MoveList *list,
    int side
)
{
    U64 knights;
    U64 own_occ;
    U64 enemy_occ;
    int piece;
    if(side == WHITE)
    {
        knights   = board->white_knights;
        own_occ   = board->white_occ;
        enemy_occ = board->black_occ;
        piece     = WHITE_KNIGHT;
    }
    else
    {
        knights   = board->black_knights;
        own_occ   = board->black_occ;
        enemy_occ = board->white_occ;
        piece     = BLACK_KNIGHT;
    }
    while(knights)
    {
        int source = get_lsb_index(knights);
        U64 attacks = knight_attacks[source] & ~own_occ;
        while(attacks)
        {
            int target = get_lsb_index(attacks);
            int capture =
            (enemy_occ &
            (1ULL << target))
            ? 1 : 0;
            add_move(
                list,
                ENCODE_MOVE(
                    source,
                    target,
                    piece,
                    0,
                    capture,
                    0,
                    0,
                    0
                )
            );
            clear_bit(&attacks, target);
        }
        clear_bit(&knights, source);
    }
}