#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "data_bus.h"

static bus_state_t *STATE = NULL;

void process_op(data_state_t*);
void check_bus(data_state_t*);

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

    d->buffer.op = NONE;
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
        if (STATE->op != NONE) {
            switch (STATE->op) {
            case IN:
                d->buffer.op = IN;
                d->buffer.addr = STATE->addr;
                break;
            case OUT:
                d->buffer.op = OUT;
                d->buffer.addr = STATE->addr;
                d->buffer.value = STATE->data_out;
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
        case IN:
            in_word(d);
            break;
        case OUT:
            out_word(d);
            break;
        }
}

void
check_bus(data_state_t *d)
{
    // check_socket();
    // examine_addr();
    // forward_req();
}

