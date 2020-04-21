#ifndef GAME_H_
#define GAME_H_
/**
 * File: game.h
 * Functions related to the game itself
 */

#include <stdio.h>
#include <stdbool.h>

#define RESET "0m"
#define CYAN  "38;5;14m"
#define BLUE  "38;5;12m"
#define RED   "38;5;9m"
#define GOLD  "38;5;220m"
#define GREEN "38;5;10m"
#define MAGEN "38;5;13m"
#define BOLD  "1m"

#define NROWS 3    // number of rows on tictactoe
#define NCOLS 3    // number of columns on tictactoe

/**
 * Initialize game @board according to the protocal layouts
 */
int init_board(char board[NROWS * NCOLS]);

/**
 * Print out current game @board onto stdout and file @f
 */
void print_board(char board[NROWS* NCOLS], FILE *f);

/**
 * Generate a move for server as player 1
 */
int gen_move(const char board[NROWS * NCOLS]);

/**
 * Make a move on the @board for @player, with @move representing the spot
 * of the move, return whether the @move is valid
 */
bool play_move(int player, int move, char board[NROWS * NCOLS]);

/**
 * Return the winning state of @board:
 * 1 or 2: player 1 or player 2
 * -1: tie
 * 0: game not finished
 */
int checkwin(char board[NROWS * NCOLS]);

/**
 * Game hlper function to print prompt message with style
 */
void prompt(const char *fmt, ...);

/**
 * Game helper function to print infomation message with style
 */
void infomsg(const char *fmt, ...);

/**
 * Game helper function to print error message with style
 */
void errmsg(const char *fmt, ...);

/**
 * Set the style of
 */
void set_style(FILE *stream, const char *style);

/**
 * Create the log file of the game
 */
FILE *create_log();

/**
 * Output to both stdout and file @f
 */
void tee(FILE *f, char const *fmt, ...);

#endif
