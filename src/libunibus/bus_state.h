#ifndef LIBBUSD_BUS_STATE_H
#define LIBBUSD_BUS_STATE_H

#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../common/include/types.h"

/* Type of data bus request to make. */
typedef enum d_req {
    R_IN,
    R_INB,
    R_OUT,
    R_OUTB,
    R_BLOCK_IN,
    R_BLOCK_INB,
    R_BLOCK_OUT,
    R_BLOCK_OUTB,
    R_DONE,
    R_NONE
} d_req_t;

/* Direction to send a message on the bus. */
typedef enum direction {
    D_LEFT,
    D_RIGHT,
    D_NONE
} direction_t;

/* (Unnecessary?) struct to hold info on data bus request to make. */
typedef struct data_op {
    d_req_t op;
    uint32_t addr;
    uint16_t value;
} data_op_t;

/* State for the data bus processor. */
typedef struct data_state {
    struct sockaddr_un d_out_addr_l;
    struct sockaddr_un d_out_addr_r;
    struct sockaddr_un d_in_addr;

    int d_bus_in;
    int d_bus_out_l;
    int d_bus_out_r;

    data_op_t buffer;

    bool_t req_issued;
    bool_t ack_received;
} data_state_t;

/* State for the priority bus processor. */
typedef struct pr_state {
    struct sockaddr_un pr_out_addr_l;
    struct sockaddr_un pr_out_addr_r;
    struct sockaddr_un pr_in_addr;

    int pr_bus_in;
    int pr_bus_out_l;
    int pr_bus_out_r;

    bool_t npr_issued;
    bool_t br_issued;
    bool_t sack_asserted;
    bool_t bbsy_asserted;
} pr_state_t;

/*
 * Shared state between bus processors and main device.
 * Grab state_mutex before using, if touching:
 * - is_master
 * - req_master
 * - rel_master
 * - shutdown
 * - l_sock
 * - sock
 * - r_sock
 */
typedef struct bus_state {
    // Shared state between buses.
    pthread_mutex_t state_mutex;
    bool_t is_master;
    bool_t req_master;
    bool_t rel_master;
    bool_t shutdown;
    char *l_sock;
    char *sock;
    char *r_sock;

    /*
     * Data bus operations.
     * - master_op_mutex protects master.
     * - slave_op_mutex protects slave.
     */
    pthread_mutex_t master_op_mutex;
    data_op_t master;

    pthread_mutex_t slave_op_mutex;
    data_op_t slave;

    /*
     * Data bus conditions.
     * - grab master_data_mutex
     *   -> signal/wait cond_master_data
     *
     * - grab slave_data_mutex
     *   -> signal/wait cond_slave_data
     */
    pthread_mutex_t master_data_mutex;
    pthread_cond_t cond_master_data;

    pthread_mutex_t slave_data_mutex;
    pthread_cond_t cond_slave_data;
} bus_state_t;

#endif
