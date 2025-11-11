#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "signals.h"
#include "data_bus.h"

static bus_state_t *STATE = NULL;

static direction_t get_fd_direction(data_state_t*, data_bus_req_t*, bool_t);
static int get_fd(data_state_t*, direction_t);
static void update_from_addr(data_state_t*, data_bus_req_t*);

void slave_wait();
void process_op(data_state_t*);

void check_bus(data_state_t*);
bool_t examine_address(data_state_t*, data_bus_req_t*);

void bus_reply(data_state_t*, data_bus_req_t*);
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

static int
get_fd(data_state_t *d, direction_t dir)
{
    switch (dir) {
    case D_LEFT:
        return d->d_bus_out_l;
    case D_RIGHT:
        return d->d_bus_out_r;
    case D_NONE:
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
init_data_state()
{
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

    return d;
}

void*
data_bus_mgr(void *bus)
{
    data_state_t *d;
    bool_t is_master;
    bool_t shutdown;
    d_req_t master_op;

    if (bus != NULL) {
        STATE = bus;
    } else {
        return NULL;
    }

    d = init_data_state();
    if (d == NULL) {
        return NULL;
    }

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
                break;
            case R_INB:
                d->buffer.op = R_INB;
                d->buffer.addr = STATE->master.addr;
                break;
            case R_OUT:
                d->buffer.op = R_OUT;
                d->buffer.addr = STATE->master.addr;
                d->buffer.value = STATE->master.value;
                break;
            case R_OUTB:
                d->buffer.op = R_OUTB;
                d->buffer.addr = STATE->master.addr;
                d->buffer.value = STATE->master.value;
                break;
            }
        }
        pthread_mutex_unlock(&(STATE->state_mutex));

        if (is_master) {
            process_op(d);
        } else {
            check_bus(d);
        }


    } while (!shutdown);
}

void
in_word(data_state_t *d)
{
    // send_msg();
    // wait_reply();
}

void
in_byte(data_state_t *d)
{
    // send_msg();
    // wait_reply();
}

void
out_word(data_state_t *d)
{
    // send_msg();
    // wait_reply();
}

void
out_byte(data_state_t *d)
{
    // send_msg();
    // wait_reply();
}

void
process_op(data_state_t *d)
{
        switch (d->buffer.op) {
        case R_IN:
            in_word(d);
            break;
        case R_INB:
            in_byte(d);
            break;
        case R_OUT:
            out_word(d);
            break;
        case R_OUTB:
            out_byte(d);
            break;
        }
}

void
check_bus(data_state_t *d)
{
    int ret;
    int err;
    bool_t is_this_device = FALSE;
    data_bus_req_t event;
    data_bus_msg_t type;

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

    if (type == DBM_REQ) {
        is_this_device = examine_address(d, &event);
        if (is_this_device) {
            bus_reply(d, &event);
        } else {
            bus_forward(d, &event);
        }
    } else {
        bus_forward(d, &event);
    }
}

bool_t
examine_address(data_state_t *d, data_bus_req_t *event)
{
    /* TODO Actually check the address. */
    return FALSE;
}

void
bus_reply(data_state_t *d, data_bus_req_t *event)
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
    // pass it on
}

void
handle_dati(data_state_t *d, data_bus_req_t *event)
{
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

    slave_wait();

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

    dir = get_fd_direction(d, event, TRUE);
    if (dir == D_NONE) {
        return;
    }

    out_fd = get_fd(d, dir);
    update_from_addr(d, &resp);
    d_connect(d, dir);
    ret = write(out_fd, &resp, sizeof(resp));
    if (ret == -1) {
        perror("handle_dati");
        return;
    }
    d_close(d, dir);
}

void
handle_dato(data_state_t *d, data_bus_req_t *event)
{
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
    if (d->d_bus_out_l == -1) {
        return -1;
    }

    d->d_bus_out_l = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (d->d_bus_out_l == -1) {
        perror("d_connect_l-socket");
        ret = -1;
        return ret;
    }

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
    if (d->d_bus_out_r == -1) {
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

    return ret;
}

