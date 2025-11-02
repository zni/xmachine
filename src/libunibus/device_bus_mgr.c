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
    bus_state_t *bus = malloc(sizeof(bus_state_t));
    if (bus == NULL) {
        perror("init_bus_malloc");
        return NULL;
    }

    bus->need_master = FALSE;
    bus->is_master = FALSE;

    bus->l_sock = l_sock;
    bus->sock = sock;
    bus->r_sock = r_sock;

    pthread_mutex_init(&(bus->state_mutex), NULL);

    return bus;
}

void
req_bus_master(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->state_mutex));
    bus->need_master = TRUE;
    pthread_mutex_unlock(&(bus->state_mutex));
}

void
release_bus_master(bus_state_t *bus)
{
    pthread_mutex_lock(&(bus->state_mutex));
    bus->need_master = FALSE;
    pthread_mutex_unlock(&(bus->state_mutex));
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

