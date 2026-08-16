#include "../include/game.h"
#include "../include/movegen.h"
#include "../include/makemove.h"
#include "../include/attacks.h"
#include "../include/board.h"
#include "../include/pieces.h"
#include "../include/side.h"
#include "../include/utils.h"
#include "../include/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* ================================================================== */
/*  Helpers                                                             */
/* ================================================================== */

const char *square_name(int sq)
{
    static char buf[3];
    buf[0] = (char)('a' + (sq % 8));
    buf[1] = (char)('1' + (sq / 8));
    buf[2] = '\0';
    return buf;
}

int parse_square(const char *s)
{
    if (!s || s[0] < 'a' || s[0] > 'h') return -1;
    if (s[1] < '1' || s[1] > '8')       return -1;
    return (s[1] - '1') * 8 + (s[0] - 'a');
}

/* Print move in short algebraic (e2e4 / e7e8q) */
static void print_move_alg(Move m)
{
    int src   = GET_SOURCE(m);
    int tgt   = GET_TARGET(m);
    int promo = GET_PROMOTED(m);

    printf("%c%d%c%d",
           'a' + (src % 8), (src / 8) + 1,
           'a' + (tgt % 8), (tgt / 8) + 1);

    if (promo)
    {
        switch (promo)
        {
            case WHITE_QUEEN:  case BLACK_QUEEN:  printf("q"); break;
            case WHITE_ROOK:   case BLACK_ROOK:   printf("r"); break;
            case WHITE_BISHOP: case BLACK_BISHOP: printf("b"); break;
            case WHITE_KNIGHT: case BLACK_KNIGHT: printf("n"); break;
            default: break;
        }
    }
}

static Move find_move(MoveList *list, int src, int tgt, int promo_piece)
{
    for (int i = 0; i < list->count; i++)
    {
        Move m = list->moves[i];
        if (GET_SOURCE(m) != src) continue;
        if (GET_TARGET(m) != tgt) continue;
        if (promo_piece)
        {
            if (GET_PROMOTED(m) == promo_piece) return m;
        }
        else
        {
            if (!GET_PROMOTED(m)) return m;
        }
    }
    return 0;
}

/* ================================================================== */
/*  Game-result check                                                   */
/* ================================================================== */
GameResult game_check_result(Board *board, int side_to_move)
{
    MoveList list = {0};
    int saved = board->side_to_move;
    board->side_to_move = side_to_move;
    generate_moves(board, &list);
    board->side_to_move = saved;

    if (list.count > 0) return GAME_ONGOING;

    U64 king_bb = (side_to_move == WHITE) ? board->white_king : board->black_king;
    int king_sq = king_bb ? get_lsb_index(king_bb) : -1;

    if (king_sq >= 0 && is_square_attacked(board, king_sq, 1 - side_to_move))
        return GAME_CHECKMATE;

    return GAME_STALEMATE;
}

/* ================================================================== */
/*  Recv thread (used only in MODE_LAN)                                */
/* ================================================================== */
static void *recv_thread_fn(void *arg)
{
    GameState *gs = (GameState *)arg;

    while (1)
    {
        Move m = net_recv_move(&gs->conn);

        pthread_mutex_lock(&gs->lock);

        if (gs->game_over)
        {
            pthread_mutex_unlock(&gs->lock);
            break;
        }

        gs->pending_move = m;
        gs->move_ready   = 1;
        pthread_cond_signal(&gs->cond);
        pthread_mutex_unlock(&gs->lock);

        if (m == 0) break;
    }
    return NULL;
}

/* ================================================================== */
/*  game_init                                                           */
/* ================================================================== */
void game_init(GameState *gs, GameMode mode, int my_side, int search_depth)
{
    memset(gs, 0, sizeof(GameState));
    init_start_position(&gs->board);

    gs->mode          = mode;
    gs->my_side       = my_side;
    gs->computer_side = (mode == MODE_VS_COMPUTER) ? (1 - my_side) : -1;
    gs->search_depth  = (search_depth > 0) ? search_depth : 4;
    gs->move_ready    = 0;
    gs->game_over     = 0;
    gs->result        = GAME_ONGOING;
    gs->draw_offered  = 0;
    gs->pending_move  = 0;

    if (mode == MODE_LAN)
    {
        pthread_mutex_init(&gs->lock, NULL);
        pthread_cond_init(&gs->cond, NULL);
    }
}

/* ================================================================== */
/*  Banner helpers                                                      */
/* ================================================================== */
static void print_banner(const GameState *gs)
{
    printf("╔═══════════════════════════════════════════════════╗\n");
    if (gs->mode == MODE_VS_COMPUTER)
    {
        printf("║      C Chess Engine — VS Minimax AI (Depth %d)    ║\n", gs->search_depth);
        printf("╠═══════════════════════════════════════════════════╣\n");
        printf("║  You play as : %-35s║\n",
               gs->my_side == WHITE ? "WHITE (uppercase)" : "BLACK (lowercase)");
        printf("║  Computer as : %-35s║\n",
               gs->computer_side == WHITE ? "WHITE (uppercase)" : "BLACK (lowercase)");
    }
    else
    {
        printf("║          C Chess Engine — 2P LAN Play             ║\n");
        printf("╠═══════════════════════════════════════════════════╣\n");
        printf("║  You play as : %-35s║\n",
               gs->my_side == WHITE ? "WHITE (uppercase)" : "BLACK (lowercase)");
    }
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║  Move   : e2e4  (source + target)                 ║\n");
    printf("║  Promo  : e7e8q  (q/r/b/n)                        ║\n");
    printf("║  Resign : resign                                  ║\n");
    if (gs->mode == MODE_LAN)
        printf("║  Draw   : draw  (offer / accept)                  ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");
}

/* ================================================================== */
/*  game_run                                                            */
/* ================================================================== */
void game_run(GameState *gs)
{
    pthread_t recv_tid;
    if (gs->mode == MODE_LAN)
    {
        pthread_create(&recv_tid, NULL, recv_thread_fn, gs);
    }

    print_banner(gs);
    print_board(&gs->board);

    while (gs->result == GAME_ONGOING)
    {
        int cur_side = gs->board.side_to_move;

        /* ============================================================ */
        /*  HUMAN PLAYER TURN                                            */
        /* ============================================================ */
        if (cur_side == gs->my_side)
        {
            printf("[%s] Your move: ", gs->my_side == WHITE ? "WHITE" : "BLACK");
            fflush(stdout);

            char input[64];
            if (fgets(input, sizeof(input), stdin) == NULL) break;
            input[strcspn(input, "\r\n")] = '\0';

            if (strcmp(input, "resign") == 0)
            {
                if (gs->mode == MODE_LAN) net_send_move(&gs->conn, (Move)NET_RESIGN);
                gs->result = GAME_RESIGN;
                printf("\nYou resigned. %s wins!\n",
                       gs->my_side == WHITE ? "Black" : "White");
                break;
            }

            if (strcmp(input, "draw") == 0)
            {
                if (gs->mode == MODE_VS_COMPUTER)
                {
                    printf("  The Computer AI declines draw offers. Keep playing!\n");
                    continue;
                }

                if (gs->draw_offered)
                {
                    net_send_move(&gs->conn, (Move)NET_DRAW_ACCEPT);
                    gs->result = GAME_DRAW_AGREED;
                    printf("\nDraw agreed!\n");
                    break;
                }
                else
                {
                    net_send_move(&gs->conn, (Move)NET_DRAW_OFFER);
                    printf("[NET] Draw offered to opponent.\n");
                    continue;
                }
            }

            if (strlen(input) < 4)
            {
                printf("  Invalid format. Use: e2e4 or e7e8q\n");
                continue;
            }

            int src = parse_square(input);
            int tgt = parse_square(input + 2);
            if (src < 0 || tgt < 0)
            {
                printf("  Invalid square. Use: e2e4\n");
                continue;
            }

            int promo = 0;
            if (strlen(input) >= 5)
            {
                char pc = input[4];
                if (gs->my_side == WHITE)
                {
                    if (pc == 'q') promo = WHITE_QUEEN;
                    else if (pc == 'r') promo = WHITE_ROOK;
                    else if (pc == 'b') promo = WHITE_BISHOP;
                    else if (pc == 'n') promo = WHITE_KNIGHT;
                }
                else
                {
                    if (pc == 'q') promo = BLACK_QUEEN;
                    else if (pc == 'r') promo = BLACK_ROOK;
                    else if (pc == 'b') promo = BLACK_BISHOP;
                    else if (pc == 'n') promo = BLACK_KNIGHT;
                }
            }

            MoveList list = {0};
            generate_moves(&gs->board, &list);

            Move chosen = find_move(&list, src, tgt, promo);
            if (!chosen)
            {
                printf("  Illegal move. Try again.\n");
                continue;
            }

            make_move(&gs->board, chosen);
            if (gs->mode == MODE_LAN) net_send_move(&gs->conn, chosen);

            printf("\n>> You played: ");
            print_move_alg(chosen);
            if (IS_CASTLING(chosen)) printf("  [castles]");
            if (IS_EP(chosen))       printf("  [en passant]");
            printf("\n");
            print_board(&gs->board);

            gs->result = game_check_result(&gs->board, gs->board.side_to_move);
            if (gs->result == GAME_CHECKMATE)
            {
                printf("  Checkmate! You win!\n");
                if (gs->mode == MODE_LAN) net_send_move(&gs->conn, (Move)NET_RESIGN);
                break;
            }
            if (gs->result == GAME_STALEMATE)
            {
                printf("  Stalemate! It's a draw!\n");
                if (gs->mode == MODE_LAN) net_send_move(&gs->conn, (Move)NET_DRAW_ACCEPT);
                break;
            }

            {
                U64 enemy_king = (gs->my_side == WHITE) ? gs->board.black_king : gs->board.white_king;
                int eks = enemy_king ? get_lsb_index(enemy_king) : -1;
                if (eks >= 0 && is_square_attacked(&gs->board, eks, gs->my_side))
                    printf("  Check!\n");
            }
        }
        /* ============================================================ */
        /*  COMPUTER AI TURN (MODE_VS_COMPUTER)                          */
        /* ============================================================ */
        else if (gs->mode == MODE_VS_COMPUTER && cur_side == gs->computer_side)
        {
            printf("[%s - AI] Thinking (depth %d)...\n",
                   gs->computer_side == WHITE ? "WHITE" : "BLACK",
                   gs->search_depth);
            fflush(stdout);

            int eval_score = 0;
            U64 nodes_searched = 0;
            Move ai_move = search_best_move(&gs->board, gs->search_depth, &eval_score, &nodes_searched);

            if (!ai_move)
            {
                gs->result = game_check_result(&gs->board, gs->board.side_to_move);
                if (gs->result == GAME_CHECKMATE) printf("  Checkmate! You win!\n");
                else printf("  Stalemate! It's a draw!\n");
                break;
            }

            make_move(&gs->board, ai_move);

            printf("\n<< Computer played: ");
            print_move_alg(ai_move);
            if (IS_CASTLING(ai_move)) printf("  [castles]");
            if (IS_EP(ai_move))       printf("  [en passant]");
            printf(" | Score: %+.2f | Nodes: %llu\n",
                   (double)eval_score / 100.0,
                   (unsigned long long)nodes_searched);

            print_board(&gs->board);

            gs->result = game_check_result(&gs->board, gs->board.side_to_move);
            if (gs->result == GAME_CHECKMATE)
            {
                printf("  Checkmate! Computer wins!\n");
                break;
            }
            if (gs->result == GAME_STALEMATE)
            {
                printf("  Stalemate! It's a draw!\n");
                break;
            }

            {
                U64 my_king = (gs->my_side == WHITE) ? gs->board.white_king : gs->board.black_king;
                int mks = my_king ? get_lsb_index(my_king) : -1;
                if (mks >= 0 && is_square_attacked(&gs->board, mks, gs->computer_side))
                    printf("  You are in check!\n");
            }
        }
        /* ============================================================ */
        /*  NETWORK OPPONENT TURN (MODE_LAN)                             */
        /* ============================================================ */
        else if (gs->mode == MODE_LAN)
        {
            printf("[%s] Waiting for network opponent...\n",
                   cur_side == WHITE ? "WHITE" : "BLACK");
            fflush(stdout);

            pthread_mutex_lock(&gs->lock);
            while (!gs->move_ready && !gs->game_over)
                pthread_cond_wait(&gs->cond, &gs->lock);

            Move opp_move   = gs->pending_move;
            gs->move_ready  = 0;
            pthread_mutex_unlock(&gs->lock);

            if (gs->game_over || opp_move == 0) break;

            if (opp_move == NET_RESIGN)
            {
                gs->result = GAME_RESIGN;
                printf("\nOpponent resigned. You win!\n");
                break;
            }
            if (opp_move == NET_DRAW_OFFER)
            {
                gs->draw_offered = 1;
                printf("\n  Opponent offers a draw. Type 'draw' to accept.\n");
                continue;
            }
            if (opp_move == NET_DRAW_ACCEPT)
            {
                gs->result = GAME_DRAW_AGREED;
                printf("\n  Draw agreed!\n");
                break;
            }

            make_move(&gs->board, opp_move);

            printf("\n<< Network opponent played: ");
            print_move_alg(opp_move);
            if (IS_CASTLING(opp_move)) printf("  [castles]");
            if (IS_EP(opp_move))       printf("  [en passant]");
            printf("\n");
            print_board(&gs->board);

            gs->result = game_check_result(&gs->board, gs->board.side_to_move);
            if (gs->result == GAME_CHECKMATE)
            {
                printf("  Checkmate! You lose!\n");
                break;
            }
            if (gs->result == GAME_STALEMATE)
            {
                printf("  Stalemate! It's a draw!\n");
                break;
            }

            {
                U64 my_king = (gs->my_side == WHITE) ? gs->board.white_king : gs->board.black_king;
                int mks = my_king ? get_lsb_index(my_king) : -1;
                if (mks >= 0 && is_square_attacked(&gs->board, mks, 1 - gs->my_side))
                    printf("  You are in check!\n");
            }
        }
    }

    /* ---- Clean up threads/sockets if LAN mode ---- */
    if (gs->mode == MODE_LAN)
    {
        pthread_mutex_lock(&gs->lock);
        gs->game_over = 1;
        pthread_cond_signal(&gs->cond);
        pthread_mutex_unlock(&gs->lock);

        net_close(&gs->conn);
        pthread_join(recv_tid, NULL);

        pthread_mutex_destroy(&gs->lock);
        pthread_cond_destroy(&gs->cond);
    }

    printf("\nGame over. Thanks for playing!\n");
}
