/* Quick verification: tests start-position move count (must be 20),
   en passant setup, castling rights display, and check detection.    */
#include "../include/board.h"
#include "../include/attacks.h"
#include "../include/movegen.h"
#include "../include/makemove.h"
#include "../include/side.h"
#include "../include/utils.h"
#include <stdio.h>
#include <assert.h>

static void test_start_position(void)
{
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    MoveList list = {0};
    generate_moves(&board, &list);

    printf("[TEST] Start position legal moves: %d (expected 20)\n", list.count);
    assert(list.count == 20 && "Start position must have exactly 20 legal moves");
    printf("[PASS] Start position\n\n");
}

static void test_en_passant(void)
{
    /* Set up: white pawn on e5, black pawn just double-pushed to d5 */
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    /* Clear default pieces, place manually */
    board.white_pawns   = 0ULL;
    board.black_pawns   = 0ULL;
    board.white_knights = board.white_bishops = board.white_rooks = 0ULL;
    board.white_queens  = board.white_king    = 0ULL;
    board.black_knights = board.black_bishops = board.black_rooks = 0ULL;
    board.black_queens  = board.black_king    = 0ULL;

    /* White pawn e5 = sq 36, white king e1 = sq 4 */
    set_bit(&board.white_pawns, 36);
    set_bit(&board.white_king,  4);
    /* Black pawn d5 = sq 35, black king e8 = sq 60 */
    set_bit(&board.black_pawns, 35);
    set_bit(&board.black_king,  60);
    board.ep_square      = 43; /* d6 = the square white can capture to */
    board.castling_rights = 0;
    board.side_to_move    = WHITE;
    update_occupancies(&board);

    MoveList list = {0};
    generate_moves(&board, &list);

    int found_ep = 0;
    for (int i = 0; i < list.count; i++)
        if (IS_EP(list.moves[i])) { found_ep = 1; break; }

    printf("[TEST] En passant move found: %s\n", found_ep ? "YES" : "NO");
    assert(found_ep && "En passant move must be generated");
    printf("[PASS] En passant\n\n");
}

static void test_castling(void)
{
    /* White: all pieces cleared, king on e1, rooks on a1 and h1 */
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    board.white_pawns   = 0ULL;
    board.white_knights = 0ULL;
    board.white_bishops = 0ULL;
    board.white_queens  = 0ULL;
    board.black_pawns   = board.black_knights = board.black_bishops = 0ULL;
    board.black_rooks   = board.black_queens  = board.black_king    = 0ULL;
    /* Keep white rooks on a1(0) and h1(7), king on e1(4) */
    board.black_king    = 0ULL;
    set_bit(&board.black_king, 60);
    board.castling_rights = CASTLE_WK | CASTLE_WQ;
    board.side_to_move    = WHITE;
    update_occupancies(&board);

    MoveList list = {0};
    generate_moves(&board, &list);

    int castles = 0;
    for (int i = 0; i < list.count; i++)
        if (IS_CASTLING(list.moves[i])) castles++;

    printf("[TEST] Castling moves available: %d (expected 2)\n", castles);
    assert(castles == 2 && "Both kingside and queenside castling must be available");
    printf("[PASS] Castling\n\n");
}

static void test_check_detection(void)
{
    /* White king on e1, black queen on e8 — white king in check from queen */
    Board board;
    init_start_position(&board);
    init_leaper_attacks();

    board.white_pawns = board.white_knights = board.white_bishops = 0ULL;
    board.white_rooks = board.white_queens  = 0ULL;
    board.black_pawns = board.black_knights = board.black_bishops = 0ULL;
    board.black_rooks = 0ULL;

    set_bit(&board.white_king,  4);  /* e1 */
    set_bit(&board.black_queens, 60); /* Black queen on e8 — attacks e-file */
    set_bit(&board.black_king,  58); /* c8 */
    board.ep_square       = -1;
    board.castling_rights = 0;
    board.side_to_move    = WHITE;
    update_occupancies(&board);

    int king_sq = get_lsb_index(board.white_king);
    int in_check = is_square_attacked(&board, king_sq, BLACK);
    printf("[TEST] White king in check from black queen on e-file: %s\n",
           in_check ? "YES" : "NO");
    assert(in_check && "White king on e1 must be in check from black queen on e8");
    printf("[PASS] Check detection\n\n");
}

int main(void)
{
    printf("=== Chess Engine Verification Tests ===\n\n");
    test_start_position();
    test_en_passant();
    test_castling();
    test_check_detection();
    printf("All tests passed!\n");
    return 0;
}
