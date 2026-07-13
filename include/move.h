#ifndef MOVE_H
#define MOVE_H
#define GET_SOURCE(move)      ((move) & 0x3F)

#define GET_TARGET(move)      (((move) >> 6) & 0x3F)

#define GET_PIECE(move)       (((move) >> 12) & 0xF)

#define GET_PROMOTED(move)    (((move) >> 16) & 0xF)

#define IS_CAPTURE(move)      (((move) >> 20) & 1)

#define IS_DOUBLE(move)       (((move) >> 21) & 1)

#define IS_EP(move)           (((move) >> 22) & 1)

#define IS_CASTLING(move)     (((move) >> 23) & 1)
#include <stdint.h>

typedef uint32_t Move;
typedef struct
{
    Move moves[256];
    int count;
    
} MoveList;
#define ENCODE_MOVE( \
source, target, piece, promoted, \
capture, double_push, enpassant, castling) \
\
((source) | \
((target) << 6) | \
((piece) << 12) | \
((promoted) << 16) | \
((capture) << 20) | \
((double_push) << 21) | \
((enpassant) << 22) | \
((castling) << 23))


void add_move(
    MoveList *list,
    Move move);
#endif