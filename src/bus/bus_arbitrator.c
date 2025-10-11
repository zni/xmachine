#include "bus_arbitrator.h"
#include "signals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unibus_t*
init_unibus()
{
    unibus_t *uni = malloc(sizeof(unibus_t));
    if (uni == NULL) {
        return NULL;
    }

    uni->data_socket = -1;
    uni->inc_data_socket = -1;

    uni->priority_socket = -1;
    uni->inc_priority_socket = -1;

    uni->init_socket = -1;
    uni->inc_init_socket = -1;

    return uni;
}

void
setup_priority_socket(unibus_t *uni)
{
    uni->priority_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (uni->priority_socket == -1) {
        perror("priority socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un p_sock;

    memset(&p_sock, 0, sizeof(p_sock));

    p_sock.sun_family = AF_UNIX;
    strncpy(p_sock.sun_path, PRIORITY_SOCKET_NAME, sizeof(p_sock.sun_path) - 1);

    int bind_status = bind(
        uni->priority_socket,
        (const struct sockaddr *) &p_sock,
        sizeof(p_sock)
    );
    if (bind_status == -1) {
        perror("priority bind");
        exit(EXIT_FAILURE);
    }
}

void
destroy_priority_socket(unibus_t *uni)
{
    unlink(PRIORITY_SOCKET_NAME);
}

void
setup_data_socket(unibus_t *uni)
{
    uni->data_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (uni->data_socket == -1) {
        perror("data socket");
        exit(EXIT_FAILURE);
    }
}

void
handle_priority_req(unibus_t *uni)
{
    if (listen(uni->priority_socket, 20) == -1) {
        perror("priority listen");
        exit(EXIT_FAILURE);
    }

    bus_req_t inc_req;
    uni->inc_priority_socket = accept(uni->priority_socket, NULL, NULL);
    if (uni->inc_priority_socket == -1) {
        perror("priority accept");
        exit(EXIT_FAILURE);
    }

    if (read(uni->inc_priority_socket, &inc_req, sizeof(bus_req_t)) == -1) {
        perror("priority read");
        exit(EXIT_FAILURE);
    }

    printf("received priority req\n");

    close(uni->inc_priority_socket);
}

int main(int argc, char **argv)
{
    unibus_t *uni = init_unibus();

    setup_priority_socket(uni);
    handle_priority_req(uni);
    destroy_priority_socket(uni);

    free(uni);

    return 0;
}

