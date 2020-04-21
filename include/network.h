#ifndef NETWORK_H_
#define NETWORK_H_
/**
 * File: network.c
 * functions for game networking
 */

#include <netinet/in.h>
#include <stdint.h>
#include <unistd.h>

#include "list.h"
#include "game.h"

#define MC_PORT  1818
#define MC_GROUP "239.0.0.1"

extern const int VERSION;

extern const int BUFSZ;
extern const int TIMEOUT;

struct session
{
    int game_id;               // unique identifer for each game
    struct sockaddr_in client; // client's socket address
    char board[NROWS * NCOLS]; // game board
    int turn;                  // current turn number
    struct list_head list;
};

struct message
{
    uint8_t version; // version number, starts at 0
    uint8_t cmd;     // connection command code
    uint8_t resp;    // response code
    uint8_t move;    // the number of the square moving to
    uint8_t turn;    // sequence number representing the turn, start with 0
    uint8_t game;    // unique identifer for game
    char board[NROWS * NCOLS];  // board for a resume game request
};

// Connection Command codes
enum Cmd
{
    MOVE  = 0,
    NGAME = 1, // new game request
    RGAME = 2, // resume game request
    NSERV = 3, // new server request
};

// Response code defined in the protocol
enum Code
{
    SUCC       = 0, // no errors
    EINVMOVE   = 1, // invalid move
    EOSYNC     = 2, // game out of sync
    EINVREQ    = 3, // invalid request
    GAMEOVR    = 4, // game over
    GAMOVRACK  = 5, // game over acknowledged
    ENOVERSION = 6, // incompatible version number
    EBUSYGAME  = 7, // server busy
    EGIDWRONG  = 8, // game ID mismatch
    SPOTAVAIL  = 9, // server spot available for multicast
};

/**
 * Create the socket based on cmd args and return the socket file descriptor
 * Set @player_no according to the cmd arg
 * Exit the program if there's error, not recoverable
 */
int init_socket(int argc, char *argv[]);

/**
 * Create the multicast socket
 * Exit the program if there's error, not recoverable
 */
int init_mc_sock();

/**
 * Initialize the session struct @s to current available game id and
 * proper sockaddr and game board
 */
int init_session(struct session *s, struct sockaddr_in addr);

/**
 * Clone a session from a resume game request based on @msg,
 * return the game number
 */
int clone_session(struct session *s, struct sockaddr_in addr,
                  struct message msg);

/**
 * Release game ID and memory of session @s
 */
void free_session(struct session *s);

/**
 * Send a message to @addr, return the return code from send call
 */
int sendmsg_to(int sockfd, struct sockaddr_in addr, struct message msg);

/**
 * Read a message from socket, set the @msg body and return the rc of recv call
 */
int recvmsg_from(
    int sockfd, struct sockaddr_in *addr, socklen_t *len, struct message *msg);

/**
 * Send a move command with possible response code to @addr
 */
int send_move(int sockfd, const struct session *sess, int move, int resp);

/**
 * Compare if two socket address is the same
 */
bool equal_addr(struct sockaddr_in lhs, struct sockaddr_in rhs);

#endif
