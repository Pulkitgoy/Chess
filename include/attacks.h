#ifndef ATTACKS_H
#define ATTACKS_H

#include "bitboard.h"
extern U64 pawn_attacks[2][64];
extern U64 knight_attacks[64];
extern U64 king_attacks[64];
U64 mask_knight_attacks(int square);

void init_leaper_attacks();

#endif