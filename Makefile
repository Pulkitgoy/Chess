CC = gcc

CFLAGS = -Wall -Wextra -O2

SRC = src/main.c \
      src/board.c \
      src/bitboard.c

TARGET = chess

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)