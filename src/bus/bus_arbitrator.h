#ifndef BUS_ARBITRATOR_H
#define BUS_ARBITRATOR_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define PRIORITY_SOCKET_NAME "/tmp/unibus_priority.socket"

typedef struct unibus {
    int data_socket;
    int inc_data_socket;

    int priority_socket;
    int inc_priority_socket;

    int init_socket;
    int inc_init_socket;
} unibus_t;

#endif

