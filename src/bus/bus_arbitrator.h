#ifndef BUS_ARBITRATOR_H
#define BUS_ARBITRATOR_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define PRIORITY_SOCKET_NAME "/tmp/xmachine/ba.socket"

typedef struct arb_state {
    int priority_socket;
    int client_socket;

    struct sockaddr_un current_client;
    socklen_t client_addr_len;

    uint8_t outstanding_bg;
    uint8_t outstanding_npg;

    uint8_t sack;
    uint8_t bbsy;
} arb_state_t;

#endif

