#include "../include/network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Platform helpers                                                    */
/* ------------------------------------------------------------------ */
#ifdef _WIN32
  static int net_init(void)
  {
      WSADATA wsa;
      if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      {
          fprintf(stderr, "[NET] WSAStartup failed: %d\n", WSAGetLastError());
          return -1;
      }
      return 0;
  }

  static void net_cleanup(void) { WSACleanup(); }

  static void close_sock(sock_t s) { closesocket(s); }
  static int  last_err(void)       { return WSAGetLastError(); }

#else
  /* POSIX fallback */
  static int net_init(void)    { return 0; }
  static void net_cleanup(void) {}
  static void close_sock(sock_t s) { close(s); }
  static int  last_err(void)   { return errno; }
#endif

/* ------------------------------------------------------------------ */
/*  net_start_server                                                    */
/* ------------------------------------------------------------------ */
int net_start_server(NetConn *conn, int port)
{
    if (net_init() != 0) return -1;

    conn->listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn->listen_sock == INVALID_SOCK)
    {
        fprintf(stderr, "[NET] socket() failed: %d\n", last_err());
        return -1;
    }

    /* Allow reuse so we can restart quickly */
    int opt = 1;
    setsockopt(conn->listen_sock, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((unsigned short)port);

    if (bind(conn->listen_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        fprintf(stderr, "[NET] bind() failed: %d\n", last_err());
        return -1;
    }

    if (listen(conn->listen_sock, 1) != 0)
    {
        fprintf(stderr, "[NET] listen() failed: %d\n", last_err());
        return -1;
    }

    printf("[NET] Server listening on port %d — waiting for opponent...\n", port);
    fflush(stdout);

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif
    conn->sock = accept(conn->listen_sock,
                        (struct sockaddr *)&client_addr, &client_len);
    if (conn->sock == INVALID_SOCK)
    {
        fprintf(stderr, "[NET] accept() failed: %d\n", last_err());
        return -1;
    }

    printf("[NET] Opponent connected from %s!\n",
           inet_ntoa(client_addr.sin_addr));
    conn->is_server = 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  net_connect                                                         */
/* ------------------------------------------------------------------ */
int net_connect(NetConn *conn, const char *ip, int port)
{
    if (net_init() != 0) return -1;

    conn->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn->sock == INVALID_SOCK)
    {
        fprintf(stderr, "[NET] socket() failed: %d\n", last_err());
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = inet_addr(ip);

    printf("[NET] Connecting to %s:%d...\n", ip, port);
    fflush(stdout);

    if (connect(conn->sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        fprintf(stderr, "[NET] connect() failed: %d\n", last_err());
        return -1;
    }

    printf("[NET] Connected!\n");
    conn->listen_sock = INVALID_SOCK;
    conn->is_server   = 0;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  net_send_move                                                       */
/* ------------------------------------------------------------------ */
int net_send_move(NetConn *conn, Move move)
{
    uint32_t m = (uint32_t)move;
    int total  = 0;
    int len    = (int)sizeof(m);

    while (total < len)
    {
        int sent = send(conn->sock, (const char *)&m + total, len - total, 0);
        if (sent <= 0)
        {
            fprintf(stderr, "[NET] send() failed: %d\n", last_err());
            return -1;
        }
        total += sent;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/*  net_recv_move                                                       */
/* ------------------------------------------------------------------ */
/*
 * Blocking receive — the recv thread calls this and sits here until
 * the opponent sends exactly 4 bytes (the Move uint32_t).
 * Returns 0 on connection close or error.
 */
Move net_recv_move(NetConn *conn)
{
    uint32_t m    = 0;
    int      total = 0;
    int      len   = (int)sizeof(m);

    while (total < len)
    {
        int r = recv(conn->sock, (char *)&m + total, len - total, 0);
        if (r <= 0) return 0; /* connection closed or error */
        total += r;
    }
    return (Move)m;
}

/* ------------------------------------------------------------------ */
/*  net_close                                                           */
/* ------------------------------------------------------------------ */
void net_close(NetConn *conn)
{
    if (conn->sock != INVALID_SOCK)
    {
        close_sock(conn->sock);
        conn->sock = INVALID_SOCK;
    }
    if (conn->listen_sock != INVALID_SOCK)
    {
        close_sock(conn->listen_sock);
        conn->listen_sock = INVALID_SOCK;
    }
    net_cleanup();
}
