# source code vpath
vpath %.h include
vpath %.c src

# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -std=gnu99 -g -Wall -I include

all: tictactoeServer

tictactoeServer: server.c network.o game.o list.h
	$(CC) $(CFLAGS) -o $@ $^

network.o: network.c network.h list.h
	$(CC) $(CFLAGS) -c $<

game.o: game.c game.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm tictactoeServer
	rm *.o
