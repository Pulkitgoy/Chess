#ifndef NETWORK_H
#define NETWORK_H

#include "move.h"

/*
 * Network layer — Winsock2 TCP for 2-player LAN chess.
 *
 * Server (White): net_start_server() → blocks until client connects.
 * Client (Black): net_connect()      → connects to server.
 *
 * Moves are sent as raw 4-byte uint32_t over the TCP stream.
 * Special sentinel values (>= 0xFFFFFFFD) signal resign / draw.
 */

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  #define INVALID_SOCK INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int sock_t;
  #define INVALID_SOCK (-1)
#endif

#define NET_RESIGN       0xFFFFFFFFU
#define NET_DRAW_OFFER   0xFFFFFFFEU
#define NET_DRAW_ACCEPT  0xFFFFFFFDU

typedef struct
{
    sock_t sock;         /* Connected peer socket            */
    sock_t listen_sock;  /* Listening socket (server only)   */
    int    is_server;
} NetConn;

/* Start a TCP server and block until one client connects.
   Returns 0 on success, -1 on error. */
int  net_start_server(NetConn *conn, int port);

/* Connect to a server at ip:port.
   Returns 0 on success, -1 on error. */
int  net_connect(NetConn *conn, const char *ip, int port);

/* Send a move (or sentinel) over the connection.
   Returns 0 on success, -1 on error. */
int  net_send_move(NetConn *conn, Move move);

/* Receive a move (blocking). Returns 0 on connection close/error. */
Move net_recv_move(NetConn *conn);

/* Close all sockets and clean up. */
void net_close(NetConn *conn);

#endif /* NETWORK_H */
