#include "../include/attacks.h"
#include "../include/side.h"
#define NOT_A_FILE  0xfefefefefefefefeULL
#define NOT_H_FILE  0x7f7f7f7f7f7f7f7fULL

#define NOT_AB_FILE 0xfcfcfcfcfcfcfcfcULL
#define NOT_GH_FILE 0x3f3f3f3f3f3f3f3fULL

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];


U64 mask_knight_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 knight = 0ULL;

    knight |= (1ULL << square);

    if ((knight >> 17) & NOT_H_FILE) attacks |= (knight >> 17);
    if ((knight >> 15) & NOT_A_FILE) attacks |= (knight >> 15);

    if ((knight >> 10) & NOT_GH_FILE) attacks |= (knight >> 10);
    if ((knight >> 6)  & NOT_AB_FILE) attacks |= (knight >> 6);

    if ((knight << 17) & NOT_A_FILE) attacks |= (knight << 17);
    if ((knight << 15) & NOT_H_FILE) attacks |= (knight << 15);

    if ((knight << 10) & NOT_AB_FILE) attacks |= (knight << 10);
    if ((knight << 6)  & NOT_GH_FILE) attacks |= (knight << 6);

    return attacks;
}

U64 mask_pawn_attacks(int side, int square)
{
    U64 attacks = 0ULL;
    U64 pawn = 0ULL;

    pawn |= (1ULL << square);

    if(side == WHITE)
    {
        if((pawn << 7) & NOT_H_FILE)
            attacks |= (pawn << 7);

        if((pawn << 9) & NOT_A_FILE)
            attacks |= (pawn << 9);
    }
    else
    {
        if((pawn >> 7) & NOT_A_FILE)
            attacks |= (pawn >> 7);

        if((pawn >> 9) & NOT_H_FILE)
            attacks |= (pawn >> 9);
    }

    return attacks;
}

U64 mask_king_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 king = 0ULL;

    king |= (1ULL << square);

    if((king << 8))
        attacks |= (king << 8);

    if((king >> 8))
        attacks |= (king >> 8);

    if((king << 1) & NOT_A_FILE)
        attacks |= (king << 1);

    if((king >> 1) & NOT_H_FILE)
        attacks |= (king >> 1);

    if((king << 9) & NOT_A_FILE)
        attacks |= (king << 9);

    if((king << 7) & NOT_H_FILE)
        attacks |= (king << 7);

    if((king >> 7) & NOT_A_FILE)
        attacks |= (king >> 7);

    if((king >> 9) & NOT_H_FILE)
        attacks |= (king >> 9);

    return attacks;
}

void init_leaper_attacks()
{
    for(int square = 0; square < 64; square++)
    {
        pawn_attacks[WHITE][square] =
            mask_pawn_attacks(WHITE, square);

        pawn_attacks[BLACK][square] =
            mask_pawn_attacks(BLACK, square);

        knight_attacks[square] =
            mask_knight_attacks(square);

        king_attacks[square] =
            mask_king_attacks(square);
    }
}