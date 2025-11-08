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

    ret = pthread_mutex_init(&(bus->data_in_mutex), NULL);
    if (ret != 0) {
        perror("data_in_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_mutex_init(&(bus->data_out_mutex), NULL);
    if (ret != 0) {
        perror("data_out_mutex");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_data_in), NULL);
    if (ret != 0) {
        perror("cond_data_in");
        free(bus);
        return NULL;
    }

    ret = pthread_cond_init(&(bus->cond_data_out), NULL);
    if (ret != 0) {
        perror("cond_data_out");
        free(bus);
        return NULL;
    }

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

void
block_data_in(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->data_in_mutex));
    pthread_cond_wait(
        &(bus->cond_data_in),
        &(bus->data_in_mutex)
    );
    pthread_mutex_unlock(&(bus->data_in_mutex));
}

void
block_data_out(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->data_out_mutex));
    pthread_cond_wait(
        &(bus->cond_data_out),
        &(bus->data_out_mutex)
    );
    pthread_mutex_unlock(&(bus->data_out_mutex));
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

