#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>

#include "network.h"
#include "game.h"

const int VERSION = 2; // current protocol version

const int BUFSZ = 100; // buffer size for all network package
const int TIMEOUT = 60; // timeout after 1 minute

#define MAX_ID 256            // maximum game ID
static int curr_max_id = 0;   // current maximum available ID
static bool used_id[MAX_ID];  // for each ID, true means it's in use

int init_socket(int argc, char *argv[])
{
    unsigned int port;
    if ((port = atoi(argv[1])) == 0 || port >= UINT16_MAX) {
        errmsg("Error: port number must be an 16-byte integer\n");
        exit(1);
    }

    // fill addr struct with getaddrinfo
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    // supplying port number
    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        errmsg("Get address failed: %s\n", gai_strerror(status));
        exit(1);
    }

    infomsg("Initializing socket as server (\033[%sX\033[%s)\n", CYAN, RESET);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        errmsg("Unable to create the socket: %s\n", strerror(errno));
        exit(1);
    }
    infomsg("Socket successfully created\n");

    // handle broken pipe locally
    signal(SIGPIPE, SIG_IGN);

    // Reuse the socket addr
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        errmsg("Unable to set socket option: %s\n", strerror(errno));
        goto sock_error;
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        errmsg("Error: Unable to bind the socket: %s\n", strerror(errno));
        goto sock_error;
    }
    infomsg("Server socket successfully binded\n");

    putchar('\n');

    freeaddrinfo(res);
    return sockfd;

sock_error: // Release resources when socket failed
    close(sockfd);
    exit(1);
}

int init_session(struct session *s, struct sockaddr_in addr)
{
    memset(s, 0, sizeof(*s));

    int game_id = -1;
    if (curr_max_id < MAX_ID) {
        game_id = curr_max_id;
        used_id[curr_max_id] = true;
        ++curr_max_id;
    } else {
        // loop to find a available ID
        for (int i = 0; i < MAX_ID; ++i) {
            if (!used_id[i]) {
                game_id = i;
                used_id[i] = true;
            }
        }
    }

    s->game_id = game_id;
    s->client = addr;
    init_board(s->board);
    s->turn = 0;

    return game_id;
}

void free_session(struct session *s)
{
    used_id[s->game_id] = false;
    free(s);
}

int sendmsg_to(int sockfd, struct sockaddr_in addr, struct message msg)
{
    int rc = sendto(
        sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, sizeof(addr));
    char buf[sizeof(addr)];

    if (rc > 0) {
      infomsg("Sent message to client %s:%u\n",
              inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(addr)),
              addr.sin_port);

      infomsg("Message content: version %d, command %d, "
              "response code %d, move %d, turn %d game %d\n",
              (int)msg.version, (int)msg.cmd, (int)msg.resp, (int)msg.move,
              (int)msg.turn, (int)msg.game);
    }

    return rc;
}

int recvmsg_from(int sockfd, struct sockaddr_in *addr, socklen_t *len,
                 struct message *msg)
{
    int rc = recvfrom(
        sockfd, msg, sizeof(*msg), 0, (struct sockaddr *)addr, len);
    char buf[*len];

    if (rc > 0) {
      infomsg("Received incoming message from %s:%u\n",
              inet_ntop(AF_INET, &addr->sin_addr, buf, *len), addr->sin_port);

      infomsg("Message content: version %d, command %d, "
              "response code %d, move %d, turn %d and game %d\n",
              (int)msg->version, (int)msg->cmd, (int)msg->resp, (int)msg->move,
              (int)msg->turn, (int)msg->game);
    }

    return rc;
}

int send_move(int sockfd, const struct session *sess, int move, int resp)
{
    struct message msg = {
        (uint8_t) VERSION,
        (uint8_t) MOVE,
        (uint8_t) resp,
        (uint8_t) move,
        (uint8_t) sess->turn,
        (uint8_t) sess->game_id,
    };

    int rc = sendmsg_to(sockfd, sess->client, msg);

    return rc;
}

bool equal_addr(struct sockaddr_in lhs, struct sockaddr_in rhs)
{
    return
        lhs.sin_addr.s_addr == rhs.sin_addr.s_addr
        && lhs.sin_port == rhs.sin_port;
}
