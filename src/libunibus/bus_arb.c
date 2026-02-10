#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "bus_arb.h"
#include "bus_state.h"
#include "debug.h"
#include "signals.h"


static arb_state_t *ARB_STATE = NULL;
static bus_state_t *BUS_STATE = NULL;

arb_state_t* init_arb(char *n_sock);
static void cleanup_arb();
static int assert_bus_grant();
static int negate_bus_grant();
static int assert_np_grant();
static int negate_np_grant();
static int handle_sack(pr_bus_req_t*);
static int handle_bbsy(pr_bus_req_t*);

static void
update_from_addr(pr_bus_req_t *req)
{
    memset(req->from, 0, sizeof(req->from));
    strncpy(req->from, ARB_STATE->current_client.sun_path, sizeof(req->from));
}

static int
client_connect()
{
    int ret;

    ARB_STATE->client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (ARB_STATE->client_socket == -1) {
        perror("client_connect");
        return -1;
    }

    ret = connect(
        ARB_STATE->client_socket,
        (const struct sockaddr *) &(ARB_STATE->current_client),
        sizeof(ARB_STATE->current_client)
    );
    if (ret == -1) {
        perror("client_connect: connect");
        return -1;
    }

    return 0;
}

static int
client_close()
{
    if (close(ARB_STATE->client_socket) == -1) {
        perror("client_close");
        return -1;
    }

    return 0;
}

arb_state_t*
init_arb(char *n_sock)
{
    ARB_STATE = malloc(sizeof(arb_state_t));
    if (ARB_STATE == NULL) {
        return NULL;
    }

    ARB_STATE->priority_socket = -1;
    ARB_STATE->client_socket = -1;

    ARB_STATE->current_client.sun_family = AF_UNIX;
    dbg_bus(BUS_STATE, "init_arb: before strncpy");
    strncpy(
        ARB_STATE->current_client.sun_path,
        n_sock,
        sizeof(ARB_STATE->current_client.sun_path) - 1
    );
    ARB_STATE->client_addr_len = sizeof(ARB_STATE->current_client);
    dbg_bus(BUS_STATE, "init_arb: after strncpy");

    ARB_STATE->sack = NEGATED;
    ARB_STATE->bbsy = NEGATED;

    ARB_STATE->outstanding_bg = FALSE;
    ARB_STATE->outstanding_npg = FALSE;

    ARB_STATE->shutdown = FALSE;

    return ARB_STATE;
}

static int
setup_arb_socket()
{
    int ret;

    ARB_STATE->priority_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (ARB_STATE->priority_socket == -1) {
        perror("priority socket");
        return -1;
    }

    memset(&ARB_STATE->socket, 0, sizeof(ARB_STATE->socket));

    ARB_STATE->socket.sun_family = AF_UNIX;
    sprintf(ARB_STATE->socket.sun_path, "/tmp/xmachine/%s_pr.socket", NAME);

    ret = bind(
        ARB_STATE->priority_socket,
        (const struct sockaddr *) &ARB_STATE->socket,
        sizeof(ARB_STATE->socket)
    );
    if (ret == -1) {
        perror("priority bind");
        return ret;
    }

    return ret;
}

static void
cleanup_arb()
{
    close(ARB_STATE->priority_socket);
    unlink(ARB_STATE->socket.sun_path);
    free(ARB_STATE);
    ARB_STATE = NULL;
}

static void
listen_priority()
{
    pr_bus_req_t req;
    int ret;
    int err;
    bool_t shutdown = FALSE;

    struct timespec wait;
    wait.tv_sec = 5;
    wait.tv_nsec = 0;

    do {
        pthread_mutex_lock(&(BUS_STATE->state_mutex));
        shutdown = BUS_STATE->shutdown;
        pthread_mutex_unlock(&(BUS_STATE->state_mutex));
        ret = recv(ARB_STATE->priority_socket, &req, sizeof(pr_bus_req_t), MSG_DONTWAIT);
        if (ret == -1) {
            err = errno;
            if (err != EAGAIN || err !=  EWOULDBLOCK) {
                perror("pr_event_read");
                return;
            }
        } else {
            switch (req.sig) {
            case BR:
                assert_bus_grant();
                break;
            case NPR:
                assert_np_grant();
                break;
            case SACK:
                handle_sack(&req);
                break;
            case BBSY:
                handle_bbsy(&req);
                break;
            default:
                break;
            }

            memset(&req, 0, sizeof(req));
        }

        nanosleep(&wait, NULL);
    } while (!shutdown);
}

static int
assert_bus_grant()
{
    int rv;
    pr_bus_req_t resp;

    if (ARB_STATE->sack == ASSERTED)
        return -1;

    ARB_STATE->outstanding_bg = TRUE;
    update_from_addr(&resp);

    client_connect();

    resp.sig = BG;
    resp.assertion = ASSERTED;
    rv = sendto(ARB_STATE->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(ARB_STATE->current_client), ARB_STATE->client_addr_len);
    if (rv == -1) {
        perror("assert_bus_grant");
        return rv;
    }

    client_close();

    return rv;
}

static int
negate_bus_grant()
{
    int rv;
    pr_bus_req_t resp;
    update_from_addr(&resp);

    ARB_STATE->outstanding_bg = FALSE;

    client_connect();

    resp.sig = BG;
    resp.assertion = NEGATED;
    rv = sendto(ARB_STATE->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(ARB_STATE->current_client), ARB_STATE->client_addr_len);
    if (rv == -1) {
        perror("negate_bus_grant");
        return rv;
    }

    client_close();

    return rv;
}

static int
assert_np_grant()
{
    int ret;
    pr_bus_req_t resp;

    if (ARB_STATE->sack == ASSERTED)
        return -1;

    update_from_addr(&resp);
    ARB_STATE->outstanding_npg = TRUE;

    client_connect();

    resp.sig = NPG;
    resp.assertion = ASSERTED;
    ret = sendto(ARB_STATE->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(ARB_STATE->current_client), ARB_STATE->client_addr_len);
    if (ret == -1) {
        perror("assert_np_grant");
        return ret;
    }

    client_close();

    return 0;
}

static int
negate_np_grant()
{
    int ret;
    pr_bus_req_t resp;

    ARB_STATE->outstanding_npg = FALSE;
    update_from_addr(&resp);

    client_connect();

    resp.sig = NPG;
    resp.assertion = NEGATED;
    ret = sendto(ARB_STATE->client_socket, &resp, sizeof(resp), 0,
                (struct sockaddr *) &(ARB_STATE->current_client), ARB_STATE->client_addr_len);
    if (ret == -1) {
        perror("negate_np_grant");
        return -1;
    }

    client_close();

    return 0;
}

static int
handle_sack(pr_bus_req_t *req)
{
    if (ARB_STATE->sack == NEGATED && req->assertion == ASSERTED) {
        ARB_STATE->sack = req->assertion;
        if (ARB_STATE->outstanding_bg == TRUE) {
            return negate_bus_grant();
        } else if (ARB_STATE->outstanding_npg == TRUE) {
            return negate_np_grant();
        }
    } else if (ARB_STATE->sack == ASSERTED && req->assertion == NEGATED) {
        ARB_STATE->sack = req->assertion;
    }

    return 0;
}

static int
handle_bbsy(pr_bus_req_t *req)
{
    if (ARB_STATE->bbsy == ASSERTED && req->assertion == NEGATED) {
        ARB_STATE->bbsy = NEGATED;
    } else if (ARB_STATE->bbsy == NEGATED && req->assertion == ASSERTED) {
        ARB_STATE->bbsy = ASSERTED;
    } else {
        return -1;
    }

    return 0;
}

void*
bus_arb_mgr(void *bus)
{
    BUS_STATE = bus;
    dbg_bus(BUS_STATE, "bus_arb_mgr: head");
    if (BUS_STATE == NULL) {
        printf("uhhh\n");
    }
    if (init_arb(NAME) == NULL) {
        return NULL;
    }
    dbg_bus(BUS_STATE, "bus_arb_mgr: after init");

    setup_arb_socket();

    pthread_mutex_lock(&(BUS_STATE->pr_ready_mutex));
    pthread_cond_signal(&(BUS_STATE->cond_pr_ready));
    pthread_mutex_unlock(&(BUS_STATE->pr_ready_mutex));

    listen_priority();
    cleanup_arb();

    return NULL;
}

