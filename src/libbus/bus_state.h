#ifndef LIBBUSD_BUS_STATE_H
#define LIBBUSD_BUS_STATE_H

#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../common/include/types.h"

typedef enum req {
    IN,
    INB,
    OUT,
    OUTB
} req_t;

typedef struct data_op {
    req_t op;
    uint32_t addr;
    uint16_t value;
} data_op_t;

typedef struct data_state {
    struct sockaddr_un data_out_addr;
    struct sockaddr_un data_in_addr;

    int data_bus_in;
    int data_bus_out;

    data_op_t buffer;
} data_state_t;

typedef struct pr_state {
    struct sockaddr_un pr_out_addr_l;
    struct sockaddr_un pr_out_addr_r;
    struct sockaddr_un pr_in_addr;

    int pr_bus_in;
    int pr_bus_out_l;
    int pr_bus_out_r;

    bool_t npr_issued;
    bool_t br_issued;
    bool_t bbsy;
    bool_t sack;
} pr_state_t;

typedef struct bus_state {
    bool_t is_master;
    bool_t need_master;

    pthread_mutex_t state_mutex;
} bus_state_t;

#endif
