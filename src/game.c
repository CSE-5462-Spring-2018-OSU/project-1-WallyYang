#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#include "game.h"

extern FILE *log_file;

const char *FILE_TEMP = "2020-OCT-01 00:00:00";
const char *TIME_FMT = "%Y-%b-%d %H:%M:%S";

void prompt(const char *fmt, ...)
{
    va_list args;

    time_t curr = time(NULL);
    struct tm *tm_time = localtime(&curr);
    char *time_str = malloc(strlen(FILE_TEMP) + 10);

    strftime(time_str, strlen(FILE_TEMP) + 10, TIME_FMT, tm_time);

    set_style(stdout, "38;5;45m");
    printf(" [%s] >>>", time_str);
    if (log_file) {
        fprintf(log_file, " [%s] >>> ", time_str);
    }
    set_style(stdout, RESET);

    set_style(stdout, BOLD);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    set_style(stdout, RESET);

    va_start(args, fmt);
    if (log_file) {
        vfprintf(log_file, fmt, args);
    }
    va_end(args);

    free(time_str);
}

void infomsg(const char *fmt, ...)
{
    va_list args;

    time_t curr = time(NULL);
    struct tm *tm_time = localtime(&curr);
    char *time_str = malloc(strlen(FILE_TEMP) + 10);

    strftime(time_str, strlen(FILE_TEMP) + 10, TIME_FMT, tm_time);

    set_style(stdout, GREEN);
    printf(" [%s] --> ", time_str);
    if (log_file) {
        fprintf(log_file, " [%s] --> ", time_str);
    }
    set_style(stdout, RESET);

    set_style(stdout, BOLD);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    set_style(stdout, RESET);

    va_start(args, fmt);
    if (log_file) {
        vfprintf(log_file, fmt, args);
    }
    va_end(args);

    free(time_str);
}

void errmsg(const char *fmt, ...)
{
    va_list args;

    set_style(stderr, RED);
    fprintf(stderr, "!!! ");
    set_style(stderr, RESET);

    set_style(stderr, BOLD);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    set_style(stderr, RESET);
}

bool play_move(int player, int move, char board[NROWS * NCOLS])
{
    char mark = (player == 1) ? 'X' : 'O';

    int row = (int)((move - 1) / NROWS);
    int col = (move - 1) % NCOLS;

    if (board[row * NCOLS +  col] == (move + '0')) {
        board[row * NCOLS + col] = mark;
        return true;
    } else {
        return false;
    }
}

int checkwin(char board[NROWS * NCOLS])
{
    char winner = '0';

    // row matches
    if (board[0] == board[1] && board[1] == board[2]) {
        winner = board[0];
        board[0] = board[1] = board[2] = tolower(board[0]);
    }

    // row matches
    else if (board[3] == board[4] && board[4] == board[5]) {
        winner = board[3];
        board[3] = board[4] = board[5] = tolower(board[3]);
    }

    // row matches
    else if (board[6] == board[7] && board[7] == board[8]) {
        winner = board[6];
        board[6] = board[7] = board[8] = tolower(board[6]);
    }

    // column
    else if (board[0] == board[3] && board[3] == board[6]) {
        winner = board[0];
        board[0] = board[3] = board[6] = tolower(board[0]);
    }

    // column
    else if (board[1] == board[4] && board[4] == board[7]) {
        winner = board[1];
        board[1] = board[4] = board[7] = tolower(board[1]);
    }

    // column
    else if (board[2] == board[5] && board[5] == board[8]) {
        winner = board[2];
        board[2] = board[5] = board[8] = tolower(board[2]);
    }

    // diagonal
    else if (board[0] == board[4] && board[4] == board[8]) {
        winner = board[0];
        board[0] = board[4] = board[8] = tolower(board[0]);
    }

    // diagonal
    else if (board[2] == board[4] && board[4] == board[6]) {
        winner = board[2];
        board[2] = board[4] = board[6] = tolower(board[2]);
    }

    // game over, tie
    else if (board[0] != '1' && board[1] != '2' && board[2] != '3' &&
             board[3] != '4' && board[4] != '5' && board[5] != '6' &&
             board[6] != '7' && board[7] != '8' && board[8] != '9')
        return -1;

    // game not finished
    else
        return 0;

    // there is winner, return the player number
    if (winner == 'X')
        return 1;
    else
        return 2;
}

/**
 * Helper function to print a cell with color
 */
static void print_cell(FILE *f, char cell)
{
    if (cell == 'X') {
        set_style(stdout, BLUE);
    } else if (cell == 'O') {
        set_style(stdout, RED);
    } else if (cell == 'x' || cell == 'o') {
        set_style(stdout, GOLD);
    }
    tee(f, "  %c  ", toupper(cell));
    set_style(stdout, RESET);
}

void print_board(char board[NROWS * NCOLS], FILE *f)
{
    /* brute force print out the board and all the squares/values    */

    printf("\n\n\n       Current TicTacToe Game\n\n");
    printf("    Player 1 (\033[%sX\033[%s)  -  Player 2 (\033[%sO\033[%s)\n\n",
           BLUE, RESET, RED, RESET);

    tee(f, "\t     |     |     \n");
    tee(f, "\t");
    print_cell(f, board[0]);
    tee(f, "|");
    print_cell(f, board[1]);
    tee(f, "|");
    print_cell(f, board[2]);
    tee(f, "\n");

    tee(f, "\t_____|_____|_____\n");
    tee(f, "\t     |     |     \n");

    tee(f, "\t");
    print_cell(f, board[3]);
    tee(f, "|");
    print_cell(f, board[4]);
    tee(f, "|");
    print_cell(f, board[5]);
    tee(f, "\n");

    tee(f, "\t_____|_____|_____\n");
    tee(f, "\t     |     |     \n");

    tee(f, "\t");
    print_cell(f, board[6]);
    tee(f, "|");
    print_cell(f, board[7]);
    tee(f, "|");
    print_cell(f, board[8]);
    tee(f, "\n");

    tee(f, "\t     |     |     \n");
    printf("\n\n\n\n");
    fprintf(f, "\n\n");
}

int init_board(char board[NROWS * NCOLS])
{
    infomsg("Initializing Game Board ...\n\n");

    int count = 1;
    for (int i = 0; i < NROWS * NCOLS; i++) {
        board[i] = count + '0';
        count++;
    }
    return 0;
}

void set_style(FILE *stream, const char *style)
{
    fprintf(stream, "\033[%s", style);
}

FILE *create_log(char **filename)
{
    time_t _time;
    struct tm *tm_time;

    time(&_time);
    tm_time = localtime(&_time);

    // +10 for extra spaces in case of long month name
    *filename = malloc(strlen(FILE_TEMP) + 10);
    if (strftime(*filename, strlen(FILE_TEMP) + 10, TIME_FMT, tm_time)) {
        infomsg("filename: %s\n", *filename);
    } else {
        errmsg("Unable to create filename!\n");
    }

    FILE *file = fopen(*filename, "w");
    if (file) {
        fprintf(file, "\n\n\n         TicTacToe Game\n\n");
        fprintf(file, "    Player 1(X)  -  Player 2(O)\n\n");
    }

    return file;
}

void tee(FILE *f, char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    if (f) {
        va_start(ap, fmt);
        vfprintf(f, fmt, ap);
        va_end(ap);
    }
}

int gen_move(const char board[NROWS * NCOLS])
{
    while (true) {
        int move = rand() % 9;
        if (board[move] != 'O' && board[move] != 'X') {
            return move + 1;
        }
    }
    return -1;
}
