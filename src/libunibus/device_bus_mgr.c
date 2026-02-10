/*
 * This is the interface for the device to interact
 * with the device's bus processes.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "bus_arb.h"
#include "data_bus.h"
#include "debug.h"
#include "device_bus_mgr.h"
#include "priority_bus.h"
#include "util.h"


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

    ret = setup_socket_dir();
    if (ret != 0) {
        fprintf(stderr, "Failed to setup socket directory\n");
        free(bus);
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

    ret = pthread_mutex_init(&(bus->perma_slave_mutex), NULL);
    if (ret != 0) {
        perror("perma_slave_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->pr_master_mutex), NULL);
    if (ret != 0) {
        perror("pr_master_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->pr_ready_mutex), NULL);
    if (ret != 0) {
        perror("pr_ready_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->data_ready_mutex), NULL);
    if (ret != 0) {
        perror("data_ready_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_pr_ready), NULL);
    if (ret != 0) {
        perror("cond_pr_ready");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_data_ready), NULL);
    if (ret != 0) {
        perror("cond_data_ready");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_pr_master), NULL);
    if (ret != 0) {
        perror("cond_pr_master");
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

    ret = pthread_cond_init(&(bus->cond_perma_slave), NULL);
    if (ret != 0) {
        perror("cond_perma_slave");
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
    dbg_bus(bus, "req_bus_master: waiting to lock...");
    pthread_mutex_lock(&(bus->state_mutex));
    dbg_bus(bus, "req_bus_master: in critical section...");
    if (bus->is_master == FALSE) {
        bus->req_master = TRUE;
    }
    pthread_mutex_unlock(&(bus->state_mutex));
    dbg_bus(bus, "req_bus_master: unlocked");

    dbg_bus(bus, "req_bus_master: waiting for master signal");
    pthread_mutex_lock(&(bus->pr_master_mutex));
    pthread_cond_wait(
        &(bus->cond_pr_master),
        &(bus->pr_master_mutex)
    );
    pthread_mutex_unlock(&(bus->pr_master_mutex));
    dbg_bus(bus, "req_bus_master: got master signal");
}

void
release_bus_master(bus_state_t *bus)
{
    dbg_bus(bus, "release_bus_master: waiting to lock...");
    pthread_mutex_lock(&(bus->state_mutex));
    dbg_bus(bus, "release_bus_master: in critical section...");
    if (bus->is_master == TRUE) {
        dbg_bus(bus, "release_bus_master: flagging release");
        bus->rel_master = TRUE;
    }
    pthread_mutex_unlock(&(bus->state_mutex));
    dbg_bus(bus, "release_bus_master: unlocked and leaving...");

    dbg_bus(bus, "release_bus_master: waiting for release signal");
    pthread_mutex_lock(&(bus->pr_rel_master_mutex));
    pthread_cond_wait(
        &(bus->cond_pr_rel_master),
        &(bus->pr_rel_master_mutex)
    );
    pthread_mutex_unlock(&(bus->pr_rel_master_mutex));
    dbg_bus(bus, "release_bus_master: got release signal");
}

uint16_t
read_data_in(bus_state_t *bus, uint32_t addr)
{
    dbg_bus(bus, "read_data_in");
    d_req_t status;
    uint16_t data;

    pthread_mutex_lock(&(bus->master_op_mutex));
    bus->master.op = R_IN;
    bus->master.addr = addr;
    pthread_mutex_unlock(&(bus->master_op_mutex));

    dbg_bus(bus, "read_data_in: R_IN set, waiting");

    pthread_mutex_lock(&(bus->master_data_mutex));
    pthread_cond_wait(
        &(bus->cond_master_data),
        &(bus->master_data_mutex)
    );
    pthread_mutex_unlock(&(bus->master_data_mutex));

    dbg_bus(bus, "read_data_in: got signal");

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

    ret = pthread_join(D_BUS, NULL);
    if (ret != 0) {
        perror("cleanup_bus_join_d");
    }
}

int
init_bus_arb(bus_state_t *bus)
{
    int ret;
    ret = pthread_create(
        &PR_BUS,
        NULL,
        &bus_arb_mgr,
        bus
    );
    if (ret != 0) {
        perror("init_bus_arb");
        return -1;
    }

    return 0;
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
connect_cpu_bus(bus_state_t *bus)
{
    bus->is_master = TRUE;

    int ret;
    ret = init_bus_arb(bus);
    if (ret != 0) {
        return ret;
    }
    pthread_mutex_lock(&(bus->pr_ready_mutex));
    pthread_cond_wait(
        &(bus->cond_pr_ready),
        &(bus->pr_ready_mutex)
    );
    pthread_mutex_unlock(&(bus->pr_ready_mutex));

    dbg_bus(bus, "connect_cpu_bus: after bus_arb");
    ret = init_d_bus(bus);
    if (ret != 0) {
        return ret;
    }
    pthread_mutex_lock(&(bus->data_ready_mutex));
    pthread_cond_wait(
        &(bus->cond_data_ready),
        &(bus->data_ready_mutex)
    );
    pthread_mutex_unlock(&(bus->data_ready_mutex));

    return 0;
}

int
connect_device_bus(bus_state_t *bus)
{
    int ret;
    ret = init_pr_bus(bus);
    if (ret != 0) {
        return ret;
    }
    pthread_mutex_lock(&(bus->pr_ready_mutex));
    pthread_cond_wait(
        &(bus->cond_pr_ready),
        &(bus->pr_ready_mutex)
    );
    pthread_mutex_unlock(&(bus->pr_ready_mutex));

    ret = init_d_bus(bus);
    if (ret != 0) {
        return ret;
    }
    pthread_mutex_lock(&(bus->data_ready_mutex));
    pthread_cond_wait(
        &(bus->cond_data_ready),
        &(bus->data_ready_mutex)
    );
    pthread_mutex_unlock(&(bus->data_ready_mutex));

    return 0;
}

void
data_bus_check(bus_state_t *bus, data_op_t *d_op)
{
    // Get slave request.
    pthread_mutex_lock(&(bus->slave_op_mutex));
    d_op->op = bus->slave.op;
    d_op->addr = bus->slave.addr;
    d_op->value = bus->slave.value;
    pthread_mutex_unlock(&(bus->slave_op_mutex));
}

void
data_bus_reply(bus_state_t *bus, data_op_t *d_op)
{
    // Update slave request.
    pthread_mutex_lock(&(bus->slave_op_mutex));
    bus->slave.op = R_DONE;
    bus->slave.addr = d_op->addr;
    bus->slave.value = d_op->value;
    d_op->op = R_NONE;
    d_op->addr = 0;
    d_op->value = 0;
    pthread_mutex_unlock(&(bus->slave_op_mutex));

    // Signal data bus.
    pthread_mutex_lock(&(bus->slave_data_mutex));
    pthread_cond_signal(&(bus->cond_slave_data));
    pthread_mutex_unlock(&(bus->slave_data_mutex));
}

