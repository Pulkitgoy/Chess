#include "../include/board.h"
#include "../include/attacks.h"
#include "../include/movegen.h"
#include "../include/makemove.h"
#include "../include/eval.h"
#include "../include/engine.h"
#include "../include/side.h"
#include "../include/utils.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_initial_eval(void)
{
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    int score = evaluate_board(&board);
    printf("[TEST] Start position static evaluation score: %d (expected 0)\n", score);
    assert(score == 0 && "Start position static evaluation must be symmetric (0)");
    printf("[PASS] Static Evaluation\n\n");
}

static void test_minimax_start_position(void)
{
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    int score = 0;
    U64 nodes = 0;
    int depth = 4;

    Move best_move = search_best_move(&board, depth, &score, &nodes);

    printf("[TEST] Minimax search (depth %d) on start position:\n", depth);
    printf("       Best Move chosen: ");
    print_move(best_move);
    printf("\n       Score: %d | Nodes evaluated: %llu\n", score, (unsigned long long)nodes);

    assert(best_move != 0 && "Minimax must return a non-zero valid move");
    assert(nodes > 0 && "Minimax must evaluate nodes");
    printf("[PASS] Minimax Search Start Position\n\n");
}

static void test_tactical_mate_in_one(void)
{
    Board board;
    memset(&board, 0, sizeof(Board));
    init_leaper_attacks();

    set_bit(&board.white_king, 4);     /* e1 */
    set_bit(&board.white_queens, 39);  /* h5 */
    set_bit(&board.white_bishops, 26); /* c4 */

    set_bit(&board.black_king, 60);    /* e8 */
    set_bit(&board.black_pawns, 53);   /* f7 */

    board.ep_square       = -1;
    board.castling_rights = 0;
    board.side_to_move    = WHITE;
    update_occupancies(&board);

    int score = 0;
    U64 nodes = 0;
    Move best_move = search_best_move(&board, 2, &score, &nodes);

    int src = GET_SOURCE(best_move);
    int tgt = GET_TARGET(best_move);

    printf("[TEST] Tactical Mate in 1 check (Scholar's Mate pattern):\n");
    printf("       Best Move chosen: %d -> %d (h5xf7#)\n", src, tgt);
    printf("       Score: %d | Nodes evaluated: %llu\n", score, (unsigned long long)nodes);

    assert(src == 39 && tgt == 53 && "AI must find 1-move checkmate h5xf7#");
    printf("[PASS] Tactical Mate in 1\n\n");
}

int main(void)
{
    printf("=== AI Minimax & Alpha-Beta Engine Verification Tests ===\n\n");
    test_initial_eval();
    test_minimax_start_position();
    test_tactical_mate_in_one();
    printf("All AI Engine tests passed successfully!\n");
    return 0;
}
