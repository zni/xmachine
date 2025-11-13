/*
 * This is the interface for the device to interact
 * with the device's bus processes.
 */

#include "device_bus_mgr.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

pthread_t PR_BUS;
pthread_t D_BUS;

bus_state_t*
init_bus(char *l_sock, char *sock, char *r_sock)
{
    int ret;
    bus_state_t *bus = malloc(sizeof(bus_state_t));
    if (bus == NULL) {
        perror("init_bus_malloc");
        return NULL;
    }

    bus->req_master = FALSE;
    bus->rel_master = FALSE;
    bus->is_master = FALSE;

    bus->l_sock = l_sock;
    bus->sock = sock;
    bus->r_sock = r_sock;

    ret = pthread_mutex_init(&(bus->state_mutex), NULL);
    if (ret != 0) {
        perror("state_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->master_op_mutex), NULL);
    if (ret != 0) {
        perror("master_op_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->slave_op_mutex), NULL);
    if (ret != 0) {
        perror("slave_op_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_slave_data), NULL);
    if (ret != 0) {
        perror("cond_slave_data");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_master_data), NULL);
    if (ret != 0) {
        perror("cond_master_data");
        free(bus);
        return NULL;
    }

    bus->master.op = R_NONE;
    bus->master.addr = 0;
    bus->master.value = 0;

    bus->slave.op = R_NONE;
    bus->slave.addr = 0;
    bus->slave.value = 0;

    return bus;
}

void
req_bus_master(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->state_mutex));
    if (bus->is_master == FALSE) {
        bus->req_master = TRUE;
    }
    pthread_mutex_unlock(&(bus->state_mutex));
}

void
release_bus_master(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->state_mutex));
    if (bus->is_master == TRUE) {
        bus->rel_master = TRUE;
    }
    pthread_mutex_unlock(&(bus->state_mutex));
}

uint16_t
read_data_in(bus_state_t *bus, uint32_t addr)
{
    d_req_t status;
    uint16_t data;

    pthread_mutex_lock(&(bus->master_op_mutex));
    bus->master.op = R_IN;
    bus->master.addr = addr;
    pthread_mutex_unlock(&(bus->master_op_mutex));

    pthread_mutex_lock(&(bus->master_data_mutex));
    pthread_cond_wait(
        &(bus->cond_master_data),
        &(bus->master_data_mutex)
    );
    pthread_mutex_unlock(&(bus->master_data_mutex));

    pthread_mutex_lock(&(bus->master_op_mutex));
    data = bus->master.value;
    status = bus->master.op;
    bus->master.op = R_NONE;
    pthread_mutex_unlock(&(bus->master_op_mutex));

    if (status != R_DONE) {
        fprintf(stderr, "read failed\n");
    }

    return data;
}

void
write_data_out(bus_state_t *bus, uint32_t addr, uint16_t data)
{
    d_req_t status;

    pthread_mutex_lock(&(bus->master_op_mutex));
    bus->master.op = R_OUT;
    bus->master.addr = addr;
    bus->master.value = data;
    pthread_mutex_unlock(&(bus->master_op_mutex));

    pthread_mutex_lock(&(bus->master_data_mutex));
    pthread_cond_wait(
        &(bus->cond_master_data),
        &(bus->master_data_mutex)
    );
    pthread_mutex_unlock(&(bus->master_data_mutex));

    pthread_mutex_lock(&(bus->master_op_mutex));
    status = bus->master.op;
    bus->master.op = R_NONE;
    pthread_mutex_unlock(&(bus->master_op_mutex));

    if (status != R_DONE) {
        fprintf(stderr, "write failed\n");
    }
}

void
cleanup_bus(bus_state_t *bus)
{
    int ret;
    pthread_mutex_lock(&(bus->state_mutex));
    bus->shutdown = TRUE;
    pthread_mutex_unlock(&(bus->state_mutex));

    ret = pthread_join(PR_BUS, NULL);
    if (ret != 0) {
        perror("cleanup_bus_join_pr");
    }

    //ret = pthread_join(D_BUS, NULL);
    //if (ret != 0) {
    //    perror("cleanup_bus_join_d");
    //}
}

int
init_pr_bus(bus_state_t *bus)
{
    int ret;
    ret = pthread_create(
        &PR_BUS,
        NULL,
        &priority_bus_mgr,
        bus
    );
    if (ret != 0) {
        perror("init_pr_bus");
        return -1;
    }

    return 0;
}

int
init_d_bus(bus_state_t *bus)
{
    int ret;
    ret = pthread_create(
        &D_BUS,
        NULL,
        &data_bus_mgr,
        bus
    );
    if (ret != 0) {
        perror("init_d_bus");
        return -1;
    }

    return 0;
}

int
connect_bus(bus_state_t *bus)
{
    int ret;
    ret = init_pr_bus(bus);
    if (ret != 0) {
        return ret;
    }

    //ret = init_d_bus(bus);
    //if (ret != 0) {
    //    return ret;
    //}

    return 0;
}

