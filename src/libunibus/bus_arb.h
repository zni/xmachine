#ifndef LIBUNIBUS_BUS_ARB_H
#define LIBUNIBUS_BUS_ARB_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "../common/include/types.h"

#define NAME "cpu"

typedef struct arb_state {
    int priority_socket;
    int client_socket;

    struct sockaddr_un socket;

    struct sockaddr_un current_client;
    socklen_t client_addr_len;

    bool_t outstanding_bg;
    bool_t outstanding_npg;

    bool_t sack;
    bool_t bbsy;

    bool_t shutdown;
} arb_state_t;

void* bus_arb_mgr(void*);

#endif

