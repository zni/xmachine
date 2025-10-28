#include <stdio.h>
#include <stdlib.h>

#include "../common/include/types.h"
#include "priority_bus.h"
#include "signals.h"

// XXX This might be common to both pr and data buses.
int get_fd_direction(pr_state_t*, bus_req_t*, bool_t);
void update_from_addr(pr_state_t*, bus_req_t*);

void handle_br(pr_state_t*, bus_req_t*);
void handle_bg(pr_state_t*, bus_req_t*);
void handle_npr(pr_state_t*, bus_req_t*);
void handle_npg(pr_state_t*, bus_req_t*);
void handle_sack(pr_state_t*, bus_req_t*);
void handle_bbsy(pr_state_t*, bus_req_t*);

pr_state_t*
init_pr_state()
{
    pr_state_t *pr = malloc(sizeof(pr_state_t));
    if (pr == NULL) {
        perror("pr_malloc");
        return NULL;
    }

    memset(&(pr->pr_out_addr_l), 0, sizeof(struct sockaddr_un));
    memset(&(pr->pr_out_addr_r), 0, sizeof(struct sockaddr_un));
    memset(&(pr->pr_in_addr), 0, sizeof(struct sockaddr_un));

    // TODO FIXME Error check all of these.
    pr->pr_bus_in = socket(AF_UNIX, SOCK_DGRAM, 0);
    pr->pr_bus_out_l = socket(AF_UNIX, SOCK_DGRAM, 0);
    pr->pr_bus_out_r = socket(AF_UNIX, SOCK_DGRAM, 0);

    pr->npr_issued = FALSE;
    pr->br_issued = FALSE;
    pr->bbsy = FALSE;
    pr->sack = FALSE;

    return pr;
}

void
priority_bus_mgr(bus_state_t *bus)
{
    bool_t is_master;
    bool_t need_master;

    pr_state_t *pr = init_pr_state();
    if (pr == NULL) {
        return;
    }

    while (TRUE) {
        pthread_mutex_lock(&(bus->state_mutex));
        is_master = bus->is_master;
        need_master = bus->need_master;
        pthread_mutex_unlock(&(bus->state_mutex));

        if (need_master && (is_master == FALSE)) {
        }
    }
}

void
send_br(pr_state_t *pr)
{
    if (pr->bbsy && pr->sack) {
        return;
    }
}

/*
 * Process a single cycle on the Unibus (one read and one write).
 */
void
process_pr_events(pr_state_t *pr)
{
    int ret;
    bus_req_t event;

    ret = read(pr->pr_bus_in, &event, sizeof(bus_req_t));
    if (ret == -1) {
        perror("pr_event_read");
        return;
    }

    switch (event.sig) {
    case BR:
        handle_br(pr, &event);
        break;
    case BG:
        handle_bg(pr, &event);
        break;
    case NPR:
        handle_npr(pr, &event);
        break;
    case NPG:
        handle_npg(pr, &event);
        break;
    case SACK:
        handle_sack(pr, &event);
        break;
    case BBSY:
        handle_bbsy(pr, &event);
        break;
    }
}

int
get_fd_direction(pr_state_t *pr, bus_req_t *req, bool_t reply)
{
    if (strncmp(req->from, pr->pr_out_addr_l.sun_path, SOCK_NAME_LEN) == 0) {
        return reply ? pr->pr_bus_out_l : pr->pr_bus_out_r;
    } else {
        return reply ? pr->pr_bus_out_r : pr->pr_bus_out_l;;
    }
}

void
update_from_addr(pr_state_t *pr, bus_req_t *req)
{
    memset(req->from, 0, sizeof(req->from));
    strncpy(req->from, pr->pr_in_addr.sun_path, sizeof(req->from));
}

void
handle_br(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd = get_fd_direction(pr, req, FALSE);

    update_from_addr(pr, req);

    ret = write(out_fd, req, sizeof(bus_req_t));
    if (ret == -1) {
        perror("handle_br_write");
        return;
    }
}

void
handle_bg(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd;

    // If we issued a bus request:
    // - BLOCK the grant.
    // - Assert a SACK.
    if (pr->br_issued) {
        bus_req_t resp;
        resp.sig = SACK;
        resp.assertion = ASSERTED;
        update_from_addr(pr, &resp);

        out_fd = get_fd_direction(pr, req, TRUE);
        ret = write(out_fd, &resp, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_bg_block_write");
            return;
        }

        // Revert br_issued, as we're no longer waiting for a BG.
        pr->br_issued = FALSE;
        pr->sack = TRUE;

    // Else PASS the grant.
    } else {
        out_fd = get_fd_direction(pr, req, FALSE);
        update_from_addr(pr, req);
        ret = write(out_fd, req, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_bg_pass_write");
            return;
        }
    }
}

void
handle_npr(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd = get_fd_direction(pr, req, FALSE);

    update_from_addr(pr, req);
    ret = write(out_fd, req, sizeof(bus_req_t));
    if (ret == -1) {
        perror("handle_npr_write");
        return;
    }
}

void
handle_npg(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd;

    // Second verse, same as the first.
    // Block grant and SACK.
    if (pr->npr_issued) {
        bus_req_t resp;
        resp.sig = SACK;
        resp.assertion = ASSERTED;
        update_from_addr(pr, &resp);

        out_fd = get_fd_direction(pr, req, TRUE);
        ret = write(out_fd, &resp, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_npg_block_write");
            return;
        }

        // Revert npr_issued, as we're no longer waiting for a NPG.
        pr->npr_issued = FALSE;
        pr->sack = TRUE;
    } else {
        // Pass the grant on.
        out_fd = get_fd_direction(pr, req, FALSE);
        update_from_addr(pr, req);
        ret = write(out_fd, req, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_npg_pass_write");
            return;
        }
    }
}

void
handle_sack(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd;

    out_fd = get_fd_direction(pr, req, FALSE);
    update_from_addr(pr, req);
    ret = write(out_fd, req, sizeof(bus_req_t));
    if (ret == -1) {
        perror("handle_npg_pass_write");
        return;
    }
}


