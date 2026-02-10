#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_bus.h"
#include "debug.h"
#include "signals.h"

static bus_state_t *STATE = NULL;

static direction_t get_fd_direction(data_state_t*, data_bus_req_t*, bool_t);
static int get_fd(data_state_t*, direction_t);
static void update_from_addr(data_state_t*, data_bus_req_t*);

void slave_wait();
void process_op(data_state_t*);

void check_bus(data_state_t*, bool_t);
void handle_resp(data_state_t*, data_bus_req_t*);
void handle_req(data_state_t*, data_bus_req_t*);

bool_t examine_address(data_state_t*, data_bus_req_t*);

/* TODO Probably just junk this. */
void bus_forward(data_state_t*, data_bus_req_t*);

void handle_dati(data_state_t*, data_bus_req_t*);
//void handle_datip(uint32_t addr);
void handle_dato(data_state_t*, data_bus_req_t*);
void handle_datob(data_state_t*, data_bus_req_t*);

int d_connect(data_state_t*, direction_t);
int d_connect_l(data_state_t*);
int d_connect_r(data_state_t*);

int d_close(data_state_t*, direction_t);
int d_close_l(data_state_t*);
int d_close_r(data_state_t*);

int send_msg(data_state_t*, data_bus_req_t*);
int wait_reply(data_state_t*, data_bus_req_t*);

static int
get_fd(data_state_t *d, direction_t dir)
{
    switch (dir) {
    case D_LEFT:
        dbg_bus(STATE, "get_fd: LEFT");
        return d->d_bus_out_l;
    case D_RIGHT:
        dbg_bus(STATE, "get_fd: RIGHT");
        return d->d_bus_out_r;
    case D_NONE:
    default:
        dbg_bus(STATE, "get_fd: NONE");
        return -1;
    }
}

static direction_t
get_fd_direction(data_state_t *d, data_bus_req_t *req, bool_t reply)
{
    if (d->d_bus_out_l == -1 && d->d_bus_out_r == -1) {
        return D_NONE;
    } else if (d->d_bus_out_l == -1) {
        if (reply) {
            return D_RIGHT;
        } else {
            return D_NONE;
        }
    } else if (d->d_bus_out_r == -1) {
        if (reply) {
            return D_LEFT;
        } else {
            return D_NONE;
        }
    } else {
        dbg_bus(STATE, "get_fd_direction: flip a coin");
        if (strncmp(req->from, d->d_out_addr_l.sun_path, SOCK_NAME_LEN) == 0) {
            return reply ? D_LEFT : D_RIGHT;
        } else {
            return reply ? D_RIGHT : D_LEFT;
        }
    }
}

static void
update_from_addr(data_state_t *d, data_bus_req_t *req)
{
    memset(req->from, 0, sizeof(req->from));
    strncpy(req->from, d->d_in_addr.sun_path, sizeof(req->from));
}

data_state_t*
init_data_state(char *l_sock, char *sock, char *r_sock)
{
    int ret;

    data_state_t *d = malloc(sizeof(data_state_t));
    if (d == NULL) {
        perror("init_data");
        return NULL;
    }

    d->req_issued = FALSE;
    d->ack_received = FALSE;

    d->buffer.op = R_NONE;
    d->buffer.addr = 0;
    d->buffer.value = 0;

    memset(&(d->d_out_addr_l), 0, sizeof(struct sockaddr_un));
    if (l_sock != NULL) {
        dbg_bus(STATE, "init_data_state: setting up l_sock");
        fprintf(stderr, "sock: %s -> l_sock: %s\n", sock, l_sock);
        d->d_out_addr_l.sun_family = AF_UNIX;
        sprintf(d->d_out_addr_l.sun_path, "/tmp/xmachine/%s_d.socket", l_sock);
    }

    memset(&(d->d_out_addr_r), 0, sizeof(struct sockaddr_un));
    if (r_sock != NULL) {
        dbg_bus(STATE, "init_data_state: setting up r_sock");
        fprintf(stderr, "sock: %s -> r_sock: %s\n", sock, r_sock);
        d->d_out_addr_r.sun_family = AF_UNIX;
        sprintf(d->d_out_addr_r.sun_path, "/tmp/xmachine/%s_d.socket", r_sock);
    }

    memset(&(d->d_in_addr), 0, sizeof(struct sockaddr_un));
    sprintf(d->d_in_addr.sun_path, "/tmp/xmachine/%s_d.socket", sock);
    d->d_in_addr.sun_family = AF_UNIX;

    d->d_bus_in = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (d->d_bus_in == -1) {
        perror("d_bus_in");
        free(d);
        return NULL;
    }

    if (l_sock == NULL) {
        d->d_bus_out_l = -1;
    }

    if (r_sock == NULL) {
        d->d_bus_out_r = -1;
    }

    ret = bind(
        d->d_bus_in,
        (const struct sockaddr *) &(d->d_in_addr),
        sizeof(d->d_in_addr)
    );
    if (ret == -1) {
        perror("d_bus_in_bind");
        free(d);
        return NULL;
    }

    return d;
}

void
d_cleanup(data_state_t *d)
{
    close(d->d_bus_in);
    unlink(d->d_in_addr.sun_path);
    free(d);
}

void*
data_bus_mgr(void *bus)
{
    struct timespec wait;
    wait.tv_sec = 5;
    wait.tv_nsec = 0;

    data_state_t *d;
    bool_t is_master;
    bool_t shutdown;
    d_req_t master_op;

    char l_sock_buf[100];
    char sock[100];
    char r_sock_buf[100];
    char *l_sock;
    char *r_sock;

    bool_t (*addr_ptr)(uint32_t) = NULL;

    if (bus != NULL) {
        STATE = bus;
    } else {
        return NULL;
    }

    pthread_mutex_lock(&(STATE->state_mutex));

    if (STATE->addr_internal_to_device != NULL) {
        addr_ptr = STATE->addr_internal_to_device;
    }

    if (STATE->l_sock != NULL && strncmp(STATE->l_sock, "ba", 2) != 0) {
        strncpy(l_sock_buf, STATE->l_sock, sizeof(l_sock_buf));
        l_sock = l_sock_buf;
    } else {
        l_sock = NULL;
    }

    strncpy(sock, STATE->sock, sizeof(sock));

    if (STATE->r_sock != NULL && strncmp(STATE->r_sock, "ba", 2) != 0) {
        strncpy(r_sock_buf, STATE->r_sock, sizeof(r_sock_buf));
        r_sock = r_sock_buf;
    } else {
        r_sock = NULL;
    }

    pthread_mutex_unlock(&(STATE->state_mutex));

    d = init_data_state(l_sock, sock, r_sock);
    if (d == NULL) {
        return NULL;
    }

    d->is_addr_internal = addr_ptr;

    pthread_mutex_lock(&(STATE->data_ready_mutex));
    pthread_cond_signal(&(STATE->cond_data_ready));
    pthread_mutex_unlock(&(STATE->data_ready_mutex));

    do {
        pthread_mutex_lock(&(STATE->state_mutex));
        is_master = STATE->is_master;
        shutdown = STATE->shutdown;
        master_op = STATE->master.op;
        if (master_op != R_NONE && master_op != R_DONE) {
            switch (master_op) {
            case R_IN:
                d->buffer.op = R_IN;
                d->buffer.addr = STATE->master.addr;
                STATE->master.op = R_NONE;
                break;
            case R_INB:
                d->buffer.op = R_INB;
                d->buffer.addr = STATE->master.addr;
                STATE->master.op = R_NONE;
                break;
            case R_OUT:
                d->buffer.op = R_OUT;
                d->buffer.addr = STATE->master.addr;
                d->buffer.value = STATE->master.value;
                STATE->master.op = R_NONE;
                break;
            case R_OUTB:
                d->buffer.op = R_OUTB;
                d->buffer.addr = STATE->master.addr;
                d->buffer.value = STATE->master.value;
                STATE->master.op = R_NONE;
                break;
            }
        }
        pthread_mutex_unlock(&(STATE->state_mutex));

        if (is_master) {
            process_op(d);
        }
        check_bus(d, is_master);
        nanosleep(&wait, NULL);
    } while (!shutdown);

    d_cleanup(d);

    return NULL;
}

void
in_word(data_state_t *d)
{
    dbg_bus(STATE, "in_word");
    int ret;
    data_bus_req_t resp;
    data_bus_req_t req;
    req.msg_type = DBM_REQ;
    req.c = D_DATI;
    req.addr = d->buffer.addr;

    update_from_addr(d, &req);
    ret = send_msg(d, &req);
    dbg_bus(STATE, "in_word: after send_msg");
    if (ret != 0) {
        /* I made this realization in reverse. Read below. */
        fprintf(stderr, "DATI: failed to send data bus message\n");
        fprintf(stderr, "DATI: this means the main thread will block forever\n");
        fprintf(stderr, "DATI: goodbye\n");
    }
//    dbg_bus(STATE, "in_word: before wait_reply");
//    ret = wait_reply(d, &resp);
//    dbg_bus(STATE, "in_word: after wait_reply");
//    if (ret != 0) {
//        /*
//         * This is the worst case Ontario.
//         * What happens now?
//         * - If we return, the main thread blocks forever.
//         * - If we signal the main thread, we've fed it lies.
//         *
//         * I should probably return for now and figure out a better solution.
//         */
//        fprintf(stderr, "DATI: failed to get reply from data bus\n");
//        fprintf(stderr, "DATI: this means the main thread will block forever\n");
//        fprintf(stderr, "DATI: goodbye\n");
//        return;
//    }
//
//    /* Put the response in the master data buffer. */
//    dbg_bus(STATE, "in_word: waiting for lock to update data buffer");
//    pthread_mutex_lock(&(STATE->master_op_mutex));
//    STATE->master.value = resp.data;
//    STATE->master.op = R_DONE;
//    pthread_mutex_unlock(&(STATE->master_op_mutex));
//    dbg_bus(STATE, "in_word: updated data buffer");
//
//    /* Assuming we miraculously got this far, signal the main thread. */
//    dbg_bus(STATE, "in_word: waiting for lock to signal main thread");
//    pthread_mutex_lock(&(STATE->master_data_mutex));
//    pthread_cond_signal(&(STATE->cond_master_data));
//    pthread_mutex_unlock(&(STATE->master_data_mutex));
//    dbg_bus(STATE, "in_word: signaled main thread, all done");
}

void
out_word(data_state_t *d)
{
    /*
     * FIXME See in_word for details on just returning below.
     */

    int ret;
    data_bus_req_t resp;
    data_bus_req_t req;
    req.msg_type = DBM_REQ;
    req.c = D_DATO;
    req.addr = d->buffer.addr;
    req.data = d->buffer.value;

    update_from_addr(d, &req);
    ret = send_msg(d, &req);
    if (ret != 0) {
        fprintf(stderr, "DATO: failed to send data bus message\n");
        fprintf(stderr, "DATO: blocking forever, goodbye.\n");
        return;
    }
    ret = wait_reply(d, &resp);
    if (ret != 0) {
        fprintf(stderr, "DATO: failed to get reply from data bus\n");
        fprintf(stderr, "DATO: blocking forever, goodbye.\n");
        return;
    }

    /* Put the response in the master data buffer. */
    pthread_mutex_lock(&(STATE->master_op_mutex));
    STATE->master.addr = 0;
    STATE->master.value = 0;
    STATE->master.op = R_DONE;
    pthread_mutex_unlock(&(STATE->master_op_mutex));

    /* Assuming we miraculously got this far, signal the main thread. */
    pthread_mutex_lock(&(STATE->master_data_mutex));
    pthread_cond_signal(&(STATE->cond_master_data));
    pthread_mutex_unlock(&(STATE->master_data_mutex));
}

void
out_byte(data_state_t *d)
{
    /*
     * FIXME See in_word for details on just returning below.
     */

    int ret;
    data_bus_req_t resp;
    data_bus_req_t req;
    req.msg_type = DBM_REQ;
    req.c = D_DATOB;
    req.addr = d->buffer.addr;
    req.data = d->buffer.value;

    update_from_addr(d, &req);
    ret = send_msg(d, &req);
    if (ret != 0) {
        fprintf(stderr, "DATOB: failed to send data bus message\n");
        fprintf(stderr, "DATOB: blocking forever, goodbye.\n");
        return;
    }
    ret = wait_reply(d, &resp);
    if (ret != 0) {
        fprintf(stderr, "DATOB: failed to get reply from data bus\n");
        fprintf(stderr, "DATOB: blocking forever, goodbye.\n");
        return;
    }

    /* Put the response in the master data buffer. */
    pthread_mutex_lock(&(STATE->master_op_mutex));
    STATE->master.addr = 0;
    STATE->master.value = 0;
    STATE->master.op = R_DONE;
    pthread_mutex_unlock(&(STATE->master_op_mutex));

    /* Assuming we miraculously got this far, signal the main thread. */
    pthread_mutex_lock(&(STATE->master_data_mutex));
    pthread_cond_signal(&(STATE->cond_master_data));
    pthread_mutex_unlock(&(STATE->master_data_mutex));
}

void
process_op(data_state_t *d)
{
        switch (d->buffer.op) {
        case R_IN:
            in_word(d);
            d->req_issued = TRUE;
            break;
        case R_OUT:
            out_word(d);
            d->req_issued = TRUE;
            break;
        case R_OUTB:
            out_byte(d);
            d->req_issued = TRUE;
            break;
        }
        d->buffer.op = R_NONE;
}

void
check_bus(data_state_t *d, bool_t is_master)
{
    int ret;
    int err;
    bool_t is_this_device = FALSE;
    data_bus_req_t event;

    ret = recv(d->d_bus_in, &event, sizeof(data_bus_req_t), MSG_DONTWAIT);
    if (ret == -1) {
        err = errno;
        if (err == EAGAIN || err ==  EWOULDBLOCK) {
            return;
        } else {
            perror("check_bus-recv");
            return;
        }
    }

    is_this_device = examine_address(d, &event);
    if (event.msg_type == DBM_REQ && is_this_device) {
        handle_req(d, &event);
    } else if (
        event.msg_type == DBM_RESP &&
        is_master &&
        d->req_issued
    ) {
        handle_resp(d, &event);
    }
    /*
     else {
        bus_forward(d, &event);
    }
    */
}

void
handle_resp(data_state_t *d, data_bus_req_t *event)
{
    dbg_bus(STATE, "handle_resp");
    switch (event->c) {
    case D_DATI:
        pthread_mutex_lock(&(STATE->master_op_mutex));
        STATE->master.value = event->data;
        STATE->master.op = R_DONE;
        pthread_mutex_unlock(&(STATE->master_op_mutex));
        break;
    default:
        return;
    }

    dbg_bus(STATE, "handle_resp: about to unblock main thread");
    pthread_mutex_lock(&(STATE->master_data_mutex));
    pthread_cond_signal(&(STATE->cond_master_data));
    pthread_mutex_unlock(&(STATE->master_data_mutex));

    d->req_issued = FALSE;
}

bool_t
examine_address(data_state_t *d, data_bus_req_t *event)
{
    if (d->is_addr_internal == NULL) {
        return FALSE;
    }

    return d->is_addr_internal(event->addr);
}

void
handle_req(data_state_t *d, data_bus_req_t *event)
{
    switch (event->c) {
    case D_DATI:
        return handle_dati(d, event);
        break;
    case D_DATIP:
        break;
    case D_DATO:
        return handle_dato(d, event);
        break;
    case D_DATOB:
        return handle_datob(d, event);
        break;
    }
}

void
bus_forward(data_state_t *d, data_bus_req_t *event)
{
    fprintf(stderr, "bus_forward has been left unimplemented. sorry.\n");
}

void
handle_dati(data_state_t *d, data_bus_req_t *event)
{
    dbg_bus(STATE, "handle_dati");

    int ret;
    int out_fd;
    direction_t dir;
    data_bus_req_t resp;
    resp.msg_type = DBM_RESP;
    resp.c = D_DATI;

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    STATE->slave.op = R_BLOCK_IN;
    STATE->slave.addr = event->addr;
    STATE->slave.value = 0;
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    dbg_bus(STATE, "handle_dati: slave_wait start");
    slave_wait();
    dbg_bus(STATE, "handle_dati: slave_wait end");

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    if (STATE->slave.op == R_DONE) {
        resp.data = STATE->slave.value;
        resp.ssyn = ASSERTED;

        STATE->slave.op = R_NONE;
        STATE->slave.addr = 0;
        STATE->slave.value = 0;
    } else {
        fprintf(stderr, "slave operation not completed. this is bad.\n");
        resp.ssyn = NEGATED;
    }
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    ret = send_msg(d, &resp);
    if (ret != 0) {
        dbg_bus(STATE, "handle_dati: failed to send message");
    }
    //dir = get_fd_direction(d, event, TRUE);
    //if (dir == D_NONE) {
    //    dbg_bus(STATE, "handle_dati: direction NONE");
    //    return;
    //}

    //out_fd = get_fd(d, dir);
    //if (out_fd <= 2) {
    //    dbg_bus(STATE, "handle_dati: not sending");
    //    return;
    //}
    //update_from_addr(d, &resp);
    //d_connect(d, dir);
    //ret = write(out_fd, &resp, sizeof(resp));
    //if (ret == -1) {
    //    perror("handle_dati");
    //    return;
    //}
    //d_close(d, dir);
}

void
handle_dato(data_state_t *d, data_bus_req_t *event)
{
    dbg_bus(STATE, "handle_dato");

    int ret;
    int out_fd;
    direction_t dir;
    data_bus_req_t resp;
    resp.msg_type = DBM_RESP;
    resp.c = D_DATO;

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    STATE->slave.op = R_BLOCK_OUT;
    STATE->slave.addr = event->addr;
    STATE->slave.value = event->data;
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    slave_wait();

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    if (STATE->slave.op == R_DONE) {
        resp.data = 0;
        resp.ssyn = ASSERTED;

        STATE->slave.op = R_NONE;
        STATE->slave.addr = 0;
        STATE->slave.value = 0;
    } else {
        fprintf(stderr, "slave operation not completed. this is bad.\n");
        resp.ssyn = NEGATED;
    }
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    dir = get_fd_direction(d, event, TRUE);
    if (dir == D_NONE) {
        return;
    }

    out_fd = get_fd(d, dir);
    update_from_addr(d, &resp);
    d_connect(d, dir);
    ret = write(out_fd, &resp, sizeof(resp));
    if (ret == -1) {
        perror("handle_dato");
        return;
    }
    d_close(d, dir);
}

void
handle_datob(data_state_t *d, data_bus_req_t *event)
{
    dbg_bus(STATE, "handle_datob");

    int ret;
    int out_fd;
    direction_t dir;
    data_bus_req_t resp;
    resp.msg_type = DBM_RESP;
    resp.c = D_DATOB;

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    STATE->slave.op = R_BLOCK_OUTB;
    STATE->slave.addr = event->addr;
    STATE->slave.value = event->data;
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    slave_wait();

    pthread_mutex_lock(&(STATE->slave_op_mutex));
    if (STATE->slave.op == R_DONE) {
        resp.data = 0;
        resp.ssyn = ASSERTED;

        STATE->slave.op = R_NONE;
        STATE->slave.addr = 0;
        STATE->slave.value = 0;
    } else {
        fprintf(stderr, "slave operation not completed. this is bad.\n");
        resp.ssyn = NEGATED;
    }
    pthread_mutex_unlock(&(STATE->slave_op_mutex));

    dir = get_fd_direction(d, event, TRUE);
    if (dir == D_NONE) {
        return;
    }

    out_fd = get_fd(d, dir);
    update_from_addr(d, &resp);
    d_connect(d, dir);
    ret = write(out_fd, &resp, sizeof(resp));
    if (ret == -1) {
        perror("handle_datob");
        return;
    }
    d_close(d, dir);
}

void
slave_wait()
{
    pthread_mutex_lock(&(STATE->perma_slave_mutex));
    pthread_cond_signal(&(STATE->cond_perma_slave));
    pthread_mutex_unlock(&(STATE->perma_slave_mutex));

    pthread_mutex_lock(&(STATE->slave_data_mutex));
    pthread_cond_wait(
        &(STATE->cond_slave_data),
        &(STATE->slave_data_mutex)
    );
    pthread_mutex_unlock(&(STATE->slave_data_mutex));
}

int
d_connect(data_state_t *d, direction_t dir)
{
    switch (dir) {
    case D_LEFT:
        return d_connect_l(d);
    case D_RIGHT:
        return d_connect_r(d);
    case D_NONE:
        return -1;
    }
}

int
d_connect_l(data_state_t *d)
{
    int ret;
    if (d->d_out_addr_l.sun_path[0] == 0) {
        return -1;
    }

    ret = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (ret == -1) {
        perror("d_connect_l-socket");
        return ret;
    }

    d->d_bus_out_l = ret;

    ret = connect(
        d->d_bus_out_l,
        (const struct sockaddr *) &(d->d_out_addr_l),
        sizeof(d->d_out_addr_l)
    );
    if (ret == -1) {
        perror("d_connect_l-connect");
        return ret;
    }

    return 0;
}

int
d_connect_r(data_state_t *d)
{
    int ret;
    if (d->d_out_addr_r.sun_path[0] == 0) {
        return -1;
    }

    d->d_bus_out_r = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (d->d_bus_out_r == -1) {
        perror("d_connect_r-socket");
        ret = -1;
        return ret;
    }

    ret = connect(
        d->d_bus_out_r,
        (const struct sockaddr *) &(d->d_out_addr_r),
        sizeof(d->d_out_addr_r)
    );
    if (ret == -1) {
        perror("d_connect_r-connect");
        return ret;
    }

    return 0;
}

int
d_close(data_state_t *d, direction_t dir)
{
    switch (dir) {
    case D_LEFT:
        return d_close_l(d);
    case D_RIGHT:
        return d_close_r(d);
    case D_NONE:
        return -1;
    }
}

int
d_close_l(data_state_t *d)
{
    int ret;
    ret = close(d->d_bus_out_l);
    if (ret == -1) {
        perror("d_close_l-close");
    }

    d->d_bus_out_l = -1;

    return ret;
}

int
d_close_r(data_state_t *d)
{
    int ret;
    ret = close(d->d_bus_out_r);
    if (ret == -1) {
        perror("d_close_r-close");
    }

    d->d_bus_out_r = -1;

    return ret;
}

int
send_msg(data_state_t *d, data_bus_req_t *req)
{
    dbg_bus(STATE, "send_msg");

    int ret;
    ret = d_connect_l(d);
    if (ret != -1) {
        ret = write(d->d_bus_out_l, req, sizeof(data_bus_req_t));
        if (ret == -1) {
            perror("send_msg_l");
            return ret;
        }
        d_close_l(d);
    }

    ret = d_connect_r(d);
    if (ret != -1) {
        ret = write(d->d_bus_out_r, req, sizeof(data_bus_req_t));
        if (ret == -1) {
            perror("send_msg_r");
            return ret;
        }
        d_close_r(d);
    }

    return 0;
}

int
wait_reply(data_state_t *d, data_bus_req_t *resp)
{
    int ret;
    ret = recv(d->d_bus_in, resp, sizeof(data_bus_req_t), MSG_DONTWAIT);
    if (ret != -1) {
        perror("wait_reply");
        return ret;
    }

    return 0;
}
