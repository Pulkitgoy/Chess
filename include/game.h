#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "move.h"
#include "network.h"
#include <pthread.h>

/*
 * Operating modes
 */
typedef enum
{
    MODE_LAN,
    MODE_VS_COMPUTER
} GameMode;

/*
 * Game-over conditions
 */
typedef enum
{
    GAME_ONGOING,
    GAME_CHECKMATE,
    GAME_STALEMATE,
    GAME_RESIGN,
    GAME_DRAW_AGREED
} GameResult;

/*
 * GameState — owns the board, mode settings, network connection, and turn-lock.
 */
typedef struct
{
    Board           board;
    NetConn         conn;
    GameMode        mode;          /* MODE_LAN or MODE_VS_COMPUTER             */
    int             my_side;       /* WHITE or BLACK (human player's color)    */
    int             computer_side; /* WHITE or BLACK (computer AI's color)     */
    int             search_depth;  /* Minimax search depth for AI (default 4)  */

    /* ---- turn lock (used in MODE_LAN) ---- */
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    volatile int    move_ready;    /* set to 1 by recv thread when move arrives */
    Move            pending_move;  /* the move received from network             */

    /* ---- game control ---- */
    volatile int    game_over;     /* set to 1 to stop recv thread               */
    GameResult      result;
    int             draw_offered;  /* 1 if opponent has offered a draw            */
} GameState;

/* Initialise game state (must call before game_run). */
void       game_init(GameState *gs, GameMode mode, int my_side, int search_depth);

/* Main game loop. Blocks until the game ends. */
void       game_run(GameState *gs);

/*
 * Check if the current position is checkmate / stalemate for side_to_move.
 * Returns GAME_CHECKMATE, GAME_STALEMATE, or GAME_ONGOING.
 */
GameResult game_check_result(Board *board, int side_to_move);

/* Helpers */
const char *square_name(int sq);      /* sq -> "e4"       */
int         parse_square(const char *s); /* "e4" -> sq, -1 on error */

#endif /* GAME_H */
