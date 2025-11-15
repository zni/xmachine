#include "bus_arbitrator.h"
#include "../libunibus/signals.h"
#include "../common/include/types.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

arb_state_t *GLOBAL_STATE = NULL;

void destroy_priority_socket(arb_state_t*);

void
handler(int signo, siginfo_t *info, void *context)
{
    if (GLOBAL_STATE != NULL) {
        if (GLOBAL_STATE->priority_socket != -1)
            destroy_priority_socket(GLOBAL_STATE);

        if (GLOBAL_STATE->client_socket != -1) {
            close(GLOBAL_STATE->client_socket);
        }

        free(GLOBAL_STATE);
    }

    exit(EXIT_SUCCESS);
}

arb_state_t*
init_arbitrator(char *n_sock)
{
    arb_state_t *arb = malloc(sizeof(arb_state_t));
    if (arb == NULL) {
        return NULL;
    }

    arb->priority_socket = -1;
    arb->client_socket = -1;

    arb->current_client.sun_family = AF_UNIX;
    strncpy(
        arb->current_client.sun_path,
        n_sock,
        sizeof(arb->current_client.sun_path) - 1
    );
    arb->client_addr_len = sizeof(arb->current_client);

    arb->sack = NEGATED;
    arb->bbsy = NEGATED;

    arb->outstanding_bg = FALSE;
    arb->outstanding_npg = FALSE;

    GLOBAL_STATE = arb;

    return arb;
}

void
client_connect(arb_state_t *arb)
{
    int ret;

    arb->client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (arb->client_socket == -1) {
        perror("client_connect");
        exit(EXIT_FAILURE);
    }

    ret = connect(
        arb->client_socket,
        (const struct sockaddr *) &(arb->current_client),
        sizeof(arb->current_client)
    );
    if (ret == -1) {
        perror("client_connect: connect");
        exit(EXIT_FAILURE);
    }
}

void
client_close(arb_state_t *arb)
{
    if (close(arb->client_socket) == -1) {
        perror("client_close");
        exit(EXIT_FAILURE);
    }
}

void
setup_priority_socket(arb_state_t *uni)
{
    int ret;
    struct sockaddr_un p_sock;

    uni->priority_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (uni->priority_socket == -1) {
        perror("priority socket");
        exit(EXIT_FAILURE);
    }

    memset(&p_sock, 0, sizeof(p_sock));

    p_sock.sun_family = AF_UNIX;
    sprintf(p_sock.sun_path, "/tmp/xmachine/%s_pr.socket", NAME);

    ret = bind(
        uni->priority_socket,
        (const struct sockaddr *) &p_sock,
        sizeof(p_sock)
    );
    if (ret == -1) {
        perror("priority bind");
        exit(EXIT_FAILURE);
    }
}

void
destroy_priority_socket(arb_state_t *arb)
{
    close(arb->priority_socket);
    unlink("/tmp/xmachine/ba_pr.socket");
}

void
dump_req(pr_bus_req_t *b)
{
    printf("REQ: ");
    switch (b->sig) {
    case BR:
        printf("BR\n");
        break;
    case BG:
        printf("BG\n");
        break;
    case NPR:
        printf("NPR\n");
        break;
    case NPG:
        printf("NPG\n");
        break;
    case SACK:
        printf("SACK\n");
        break;
    case BBSY:
        printf("BBSY\n");
        break;
    }

    printf("ASSERTION: %s\n", b->assertion ? "ASSERTED" : "NEGATED");
}

void
assert_bus_grant(arb_state_t *arb)
{
    int rv;
    pr_bus_req_t resp;

    if (arb->sack == ASSERTED)
        return;

    arb->outstanding_bg = TRUE;

    client_connect(arb);

    resp.sig = BG;
    resp.assertion = ASSERTED;
    rv = sendto(arb->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(arb->current_client), arb->client_addr_len);
    if (rv == -1) {
        perror("assert_bus_grant");
        exit(EXIT_FAILURE);
    }

    client_close(arb);
}

void
negate_bus_grant(arb_state_t *arb)
{
    printf("negate_bus_grant\n");
    int rv;
    pr_bus_req_t resp;

    arb->outstanding_bg = FALSE;

    client_connect(arb);

    resp.sig = BG;
    resp.assertion = NEGATED;
    rv = sendto(arb->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(arb->current_client), arb->client_addr_len);
    if (rv == -1) {
        perror("negate_bus_grant");
        exit(EXIT_FAILURE);
    }

    client_close(arb);
}

void
assert_np_grant(arb_state_t *arb)
{
    int rv;
    pr_bus_req_t resp;

    if (arb->sack == ASSERTED)
        return;

    arb->outstanding_npg = TRUE;

    client_connect(arb);

    resp.sig = NPG;
    resp.assertion = ASSERTED;
    rv = sendto(arb->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(arb->current_client), arb->client_addr_len);
    if (rv == -1) {
        perror("negate_bus_grant");
        exit(EXIT_FAILURE);
    }

    client_close(arb);
}

void
negate_np_grant(arb_state_t *arb)
{
    int rv;
    pr_bus_req_t resp;

    arb->outstanding_npg = FALSE;

    client_connect(arb);

    resp.sig = NPG;
    resp.assertion = NEGATED;
    rv = sendto(arb->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(arb->current_client), arb->client_addr_len);
    if (rv == -1) {
        perror("negate_bus_grant");
        exit(EXIT_FAILURE);
    }

    client_close(arb);
}

void
handle_sack(arb_state_t *arb, pr_bus_req_t *req)
{
    if (arb->sack == NEGATED && req->assertion == ASSERTED) {
        arb->sack = req->assertion;
        if (arb->outstanding_bg == TRUE) {
            negate_bus_grant(arb);
        } else if (arb->outstanding_npg == TRUE) {
            negate_np_grant(arb);
        }
    } else if (arb->sack == ASSERTED && req->assertion == NEGATED) {
        arb->sack = req->assertion;
    }
}

void
handle_bbsy(arb_state_t *arb, pr_bus_req_t *req)
{
    if (arb->bbsy == ASSERTED && req->assertion == NEGATED) {
        arb->bbsy = NEGATED;
    } else if (arb->bbsy == NEGATED && req->assertion == ASSERTED) {
        arb->bbsy = ASSERTED;
    }
}

void
listen_priority(arb_state_t *arb)
{
    pr_bus_req_t req;
    int ret;

    printf("listening...\n");
    while (TRUE) {
        ret = recv(arb->priority_socket, &req, sizeof(pr_bus_req_t), MSG_WAITALL);
        if (ret == -1) {
            perror("priority recv");
            exit(EXIT_FAILURE);
        }

        printf("received priority req\n");
        dump_req(&req);

        switch (req.sig) {
        case BR:
            assert_bus_grant(arb);
            break;
        case NPR:
            assert_np_grant(arb);
            break;
        case SACK:
            handle_sack(arb, &req);
            break;
        case BBSY:
            handle_bbsy(arb, &req);
            break;
        default:
            break;
        }

        memset(&req, 0, sizeof(req));
    }
}

int
main(int argc, char **argv)
{
    struct sigaction act = { 0 };

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler;
    if (sigaction(SIGHUP, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    fprintf(stderr, "bus arbitrator starting [%d]\n", pid);

    arb_state_t *arb = init_arbitrator("/tmp/xmachine/cpu_pr.socket");

    setup_priority_socket(arb);
    listen_priority(arb);
    destroy_priority_socket(arb);

    free(arb);

    return 0;
}

