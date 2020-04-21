/**
 * File: client.c
 * This is the client program for the client/server tictactoe game
 * acting as player 2
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "list.h"
#include "game.h"
#include "network.h"

FILE *log_file = NULL;

static bool sigint = false;
void exit_handler(int s);

int main(int argc, char *argv[])
{
    int rc; // general return codes

    srand(time(NULL));

    set_style(stdout, "\033[2J\033[H");
    fflush(stdout);
    if (argc < 2) {
        errmsg("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // handling SIGINT locally
    struct sigaction sigint_handler;
    sigint_handler.sa_handler = exit_handler;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;

    sigaction(SIGINT, &sigint_handler, NULL);

    int sockfd = init_socket(argc, argv);
    log_file = fopen("server.log", "a");
    if (!log_file) {
        goto error;
    }
    fprintf(log_file, "\n\n");

    // create linked list of sessions
    LIST_HEAD(list_session);

    do {
        // server running

        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        struct message msg;
        char buf[addr_len];

        tee(log_file, "\n");
        rc = recvmsg_from(sockfd, &addr, &addr_len, &msg);
        if (rc < 0) {
            errmsg("Unable to receive message, retry: %s\n", strerror(errno));
            continue;
        }

        if (msg.cmd == NGAME) { // new game request
            infomsg("NEW GAME request from %s:%u\n",
                    inet_ntop(AF_INET, &addr.sin_addr, buf, addr_len),
                    addr.sin_port);

            bool found = false;
            struct session *pos;
            list_for_each_entry(pos, &list_session, list) {
                if (equal_addr(pos->client, addr)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                errmsg("Existing client sent new game request, rejecting\n");
                rc = send_move(sockfd, pos, 0, EBUSYGAME);
                if (rc <= 0) {
                  errmsg("Unable to send response message: %s\n",
                         strerror(errno));
                }
                continue;
            }

            struct session *sess = malloc(sizeof(struct session));
            rc = init_session(sess, addr);
            if (rc < 0) {
                errmsg("Server at full load, send busy response code\n");
                rc = send_move(sockfd, sess, 0, EBUSYGAME);
                if (rc <= 0) {
                    errmsg("Unable to send response message: %s\n",
                           strerror(errno));
                }
                continue;
            }

            int move = gen_move(sess->board);
            play_move(1, move, sess->board);
            print_board(sess->board, log_file);

            infomsg("Assigned game ID %d to client %s:%u\n",
                    sess->game_id,
                    inet_ntop(AF_INET, &addr.sin_addr, buf, addr_len),
                    addr.sin_port);
            rc = send_move(sockfd, sess, move, SUCC);
            if (rc <= 0) {
                errmsg("Unable to send initial message: %s\n", strerror(errno));
                continue;
            }

            INIT_LIST_HEAD(&sess->list);
            list_add(&sess->list, &list_session);
            infomsg("Added session to the current list\n");

            continue;
        }

        struct session *sess = NULL;
        list_for_each_entry(sess, &list_session, list) {
            if (equal_addr(addr, sess->client)) {
                infomsg("New message from current session\n");

                // check for game ID
                if (msg.game != sess->game_id) {
                    errmsg("Received mismatched game ID, expected %d, got %d\n",
                           sess->game_id, msg.game);
                    rc = send_move(sockfd, sess, 0, EGIDWRONG);
                    if (rc <= 0) {
                        errmsg("Unable to send response message: %s\n",
                               strerror(errno));
                    }
                    break;
                }

                if (msg.resp != SUCC
                    && msg.resp != GAMEOVR
                    && msg.resp != GAMOVRACK) {

                    // error not able to handle
                    break;
                }

                int winner = 0;

                set_style(stdout, "\033[2J\033[H");
                fflush(stdout);

                if (play_move(2, msg.move, sess->board)) {
                    winner = checkwin(sess->board);
                    print_board(sess->board, log_file);
                } else {
                    errmsg("Received invalid move, send back response\n");
                    rc = send_move(sockfd, sess, 0, EINVMOVE);
                    if (rc <= 0) {
                        errmsg("Unable to send response message: %s\n",
                               strerror(errno));
                    }
                    break;
                }

                ++(sess->turn);

                if (winner != 0) {
                    infomsg("Server lost\n");
                    // send acknowledge
                    rc = send_move(sockfd, sess, 0, GAMOVRACK);

                    // remove session from the list
                    list_del(&sess->list);
                    free_session(sess);
                    break;
                }

                int move = gen_move(sess->board);
                play_move(1, move, sess->board);
                ++(sess->turn);

                winner = checkwin(sess->board);
                print_board(sess->board, log_file);

                if (winner == 0) {
                    infomsg("Sending move to client\n");
                    rc = send_move(sockfd, sess, move, SUCC);
                    if (rc <= 0) {
                        errmsg("Failed to send message to client: %s\n",
                               strerror(errno));
                    }
                } else {
                    infomsg("Sending move with winning message\n");
                    rc = send_move(sockfd, sess, move, GAMEOVR);
                    if (rc <= 0) {
                        errmsg("Failed to send message to client: %s\n",
                               strerror(errno));
                    }

                    // remove session from the list
                    list_del(&sess->list);
                    free_session(sess);
                    break;
                }
            }
        }
    } while (!sigint);

    infomsg("Server stopped, clean up resources and exit\n");

    close(sockfd);
    infomsg("Socket closed\n");

    if (log_file) {
        fflush(log_file);
        fclose(log_file);
        infomsg("Log file writes out, closing the stream\n");
        infomsg("Open server.log for infomation\n");
    }
    infomsg("All resources cleared, shutdown now\n");

    return 0;

error:
    if (log_file != NULL) {
        fflush(log_file);
        fclose(log_file);
    }
    errmsg("Encountered internal error!\n");
    errmsg("Clean up resources and shutdown\n");
    close(sockfd);
    errmsg("Resouces cleared, closing game\n");

    return 1;
}

void exit_handler(int s)
{
    putchar('\n');
    infomsg("Received SIGINT %d\n", s);
    sigint = true;
}
