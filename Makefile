CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lws2_32 -lpthread

SRC = src/main.c        \
      src/board.c       \
      src/bitboard.c    \
      src/attacks.c     \
      src/movegen.c     \
      src/makemove.c    \
      src/sliding.c     \
      src/move.c        \
      src/utils.c       \
      src/board_utils.c \
      src/network.c     \
      src/game.c        \
      src/eval.c        \
      src/engine.c

TARGET = chess

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe