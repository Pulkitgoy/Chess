#include "../include/board.h"
#include "../include/attacks.h"
#include "../include/game.h"
#include "../include/side.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static void usage(const char *prog)
{
    fprintf(stderr,
        "Chess Engine Usage:\n\n"
        "  1. Play vs Computer AI:\n"
        "     %s ai [white|black] [depth]\n"
        "     Example: %s ai white 4\n\n"
        "  2. LAN 2-Player Mode:\n"
        "     Server (White): %s server <port>\n"
        "     Client (Black): %s client <ip> <port>\n"
        "     Example: %s server 5000  |  %s client 127.0.0.1 5000\n\n"
        "  3. Interactive Menu:\n"
        "     %s\n",
        prog, prog, prog, prog, prog, prog, prog);
}

static void run_interactive_menu(void)
{
    printf("=======================================\n");
    printf("         C CHESS ENGINE (AI & LAN)     \n");
    printf("=======================================\n\n");
    printf("  1. Single Player (VS Minimax AI)\n");
    printf("  2. LAN Multiplayer — Host Server (White)\n");
    printf("  3. LAN Multiplayer — Connect Client (Black)\n\n");
    printf("Select mode (1-3): ");
    fflush(stdout);

    char line[64];
    if (!fgets(line, sizeof(line), stdin)) return;
    int choice = atoi(line);

    GameState gs;

    if (choice == 1)
    {
        printf("\nChoose your side:\n");
        printf("  1. White (you move first)\n");
        printf("  2. Black (computer moves first)\n");
        printf("Select side (1-2) [default 1]: ");
        fflush(stdout);

        int side = WHITE;
        if (fgets(line, sizeof(line), stdin))
        {
            int side_choice = atoi(line);
            if (side_choice == 2) side = BLACK;
        }

        printf("Enter AI search depth (1-6) [default 4]: ");
        fflush(stdout);
        int depth = 4;
        if (fgets(line, sizeof(line), stdin))
        {
            int user_depth = atoi(line);
            if (user_depth >= 1 && user_depth <= 8) depth = user_depth;
        }

        game_init(&gs, MODE_VS_COMPUTER, side, depth);
        game_run(&gs);
    }
    else if (choice == 2)
    {
        printf("Enter port to listen on [default 5000]: ");
        fflush(stdout);
        int port = 5000;
        if (fgets(line, sizeof(line), stdin))
        {
            int p = atoi(line);
            if (p > 0) port = p;
        }

        game_init(&gs, MODE_LAN, WHITE, 4);
        if (net_start_server(&gs.conn, port) != 0)
        {
            fprintf(stderr, "Failed to start server on port %d.\n", port);
            return;
        }
        game_run(&gs);
    }
    else if (choice == 3)
    {
        printf("Enter Server IP [default 127.0.0.1]: ");
        fflush(stdout);
        char ip[64] = "127.0.0.1";
        if (fgets(line, sizeof(line), stdin))
        {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) > 0) strcpy(ip, line);
        }

        printf("Enter Server Port [default 5000]: ");
        fflush(stdout);
        int port = 5000;
        if (fgets(line, sizeof(line), stdin))
        {
            int p = atoi(line);
            if (p > 0) port = p;
        }

        game_init(&gs, MODE_LAN, BLACK, 4);
        if (net_connect(&gs.conn, ip, port) != 0)
        {
            fprintf(stderr, "Failed to connect to %s:%d.\n", ip, port);
            return;
        }
        game_run(&gs);
    }
    else
    {
        printf("Invalid choice. Exiting.\n");
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);  /* Switch console to UTF-8 so box-drawing chars render correctly */
#endif
    init_leaper_attacks();

    if (argc < 2)
    {
        run_interactive_menu();
        return 0;
    }

    GameState gs;

    if (strcmp(argv[1], "ai") == 0)
    {
        int my_side = WHITE;
        int depth = 4;

        if (argc >= 3)
        {
            if (strcmp(argv[2], "black") == 0) my_side = BLACK;
        }
        if (argc >= 4)
        {
            int d = atoi(argv[3]);
            if (d >= 1 && d <= 8) depth = d;
        }

        game_init(&gs, MODE_VS_COMPUTER, my_side, depth);
        game_run(&gs);
    }
    else if (strcmp(argv[1], "server") == 0)
    {
        if (argc < 3)
        {
            usage(argv[0]);
            return 1;
        }
        int port = atoi(argv[2]);
        if (port <= 0)
        {
            fprintf(stderr, "Invalid port: %s\n", argv[2]);
            return 1;
        }
        game_init(&gs, MODE_LAN, WHITE, 4);
        if (net_start_server(&gs.conn, port) != 0)
        {
            fprintf(stderr, "Failed to start server.\n");
            return 1;
        }
        game_run(&gs);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        if (argc < 4)
        {
            usage(argv[0]);
            return 1;
        }
        const char *ip   = argv[2];
        int          port = atoi(argv[3]);
        if (port <= 0)
        {
            fprintf(stderr, "Invalid port: %s\n", argv[3]);
            return 1;
        }
        game_init(&gs, MODE_LAN, BLACK, 4);
        if (net_connect(&gs.conn, ip, port) != 0)
        {
            fprintf(stderr, "Failed to connect.\n");
            return 1;
        }
        game_run(&gs);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        usage(argv[0]);
        return 1;
    }

    return 0;
}