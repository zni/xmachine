#include <stdio.h>
#include <stdlib.h>

#include "../common/include/types.h"
#include "priority_bus.h"
#include "signals.h"

bus_state_t *STATE = NULL;

int get_fd_direction(pr_state_t*, bus_req_t*, bool_t);
void update_from_addr(pr_state_t*, bus_req_t*);

void assert_br(pr_state_t*);
void assert_bbsy(pr_state_t*);

void process_pr_events(pr_state_t *pr);

void handle_br(pr_state_t*, bus_req_t*);
void handle_bg(pr_state_t*, bus_req_t*);
void handle_npr(pr_state_t*, bus_req_t*);
void handle_npg(pr_state_t*, bus_req_t*);
void handle_sack(pr_state_t*, bus_req_t*);
void handle_bbsy(pr_state_t*, bus_req_t*);

pr_state_t*
init_pr_state(char *l_sock, char *sock, char *r_sock)
{
    int ret;

    pr_state_t *pr = malloc(sizeof(pr_state_t));
    if (pr == NULL) {
        perror("pr_malloc");
        return NULL;
    }

    if (l_sock != NULL) {
        memset(&(pr->pr_out_addr_l), 0, sizeof(struct sockaddr_un));
        strncpy(pr->pr_out_addr_l.sun_path, l_sock, sizeof(pr->pr_out_addr_l.sun_path));
    }

    if (r_sock != NULL) {
        memset(&(pr->pr_out_addr_r), 0, sizeof(struct sockaddr_un));
        strncpy(pr->pr_out_addr_r.sun_path, r_sock, sizeof(pr->pr_out_addr_r.sun_path));
    }

    memset(&(pr->pr_in_addr), 0, sizeof(struct sockaddr_un));
    strncpy(pr->pr_in_addr.sun_path, sock, sizeof(pr->pr_in_addr.sun_path));

    pr->pr_bus_in = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (pr->pr_bus_in == -1) {
        perror("pr_bus_in");
        free(pr);
        return NULL;
    }

    if (l_sock != NULL) {
        pr->pr_bus_out_l = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (pr->pr_bus_out_l == -1) {
            perror("pr_bus_out_l");
            free(pr);
            return NULL;
        }
    } else {
        pr->pr_bus_out_l = -1;
    }

    if (r_sock != NULL) {
        pr->pr_bus_out_r = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (pr->pr_bus_out_r == -1) {
            perror("pr_bus_out_r");
            free(pr);
            return NULL;
        }
    } else {
        pr->pr_bus_out_r = -1;
    }

    ret = bind(
        pr->pr_bus_in,
        (const struct sockaddr *) &(pr->pr_in_addr),
        sizeof(pr->pr_in_addr)
    );
    if (ret == -1) {
        perror("pr_bus_in_bind");
        free(pr);
        return NULL;
    }

    if (pr->pr_bus_out_l != -1) {
        ret = connect(
            pr->pr_bus_out_l,
            (const struct sockaddr *) &(pr->pr_out_addr_l),
            sizeof(pr->pr_out_addr_l)
        );
        if (ret == -1) {
            perror("pr_bus_out_l_connect");
            free(pr);
            return NULL;
        }
    }

    if (pr->pr_bus_out_r != -1) {
        ret = connect(
            pr->pr_bus_out_r,
            (const struct sockaddr *) &(pr->pr_out_addr_r),
            sizeof(pr->pr_out_addr_r)
        );
        if (ret == -1) {
            perror("pr_bus_out_r_connect");
            free(pr);
            return NULL;
        }
    }

    pr->npr_issued = FALSE;
    pr->br_issued = FALSE;
    pr->bbsy_asserted = FALSE;
    pr->sack_asserted = FALSE;

    return pr;
}

void
priority_bus_mgr(void *bus)
{
    bool_t is_master;
    bool_t need_master;
    char l_sock_buf[100];
    char sock[100];
    char r_sock_buf[100];
    char *l_sock;
    char *r_sock;

    if (bus == NULL) {
        return;
    } else {
        STATE = bus;
    }

    pthread_mutex_lock(&(STATE->state_mutex));

    if (STATE->l_sock != NULL) {
        strncpy(l_sock_buf, STATE->l_sock, sizeof(l_sock_buf));
        l_sock = l_sock_buf;
    } else {
        l_sock = NULL;
    }

    strncpy(sock, STATE->sock, sizeof(sock));

    if (STATE->r_sock != NULL) {
        strncpy(r_sock_buf, STATE->r_sock, sizeof(r_sock_buf));
        r_sock = r_sock_buf;
    } else {
        r_sock = NULL;
    }

    pthread_mutex_unlock(&(STATE->state_mutex));

    pr_state_t *pr = init_pr_state(l_sock, sock, r_sock);
    if (pr == NULL) {
        return;
    }

    while (TRUE) {
        pthread_mutex_lock(&(STATE->state_mutex));
        is_master = STATE->is_master;
        need_master = STATE->need_master;
        pthread_mutex_unlock(&(STATE->state_mutex));

        if (need_master && (is_master == FALSE)) {
            if (pr->bbsy_asserted == FALSE) {
                assert_br(pr);
            }
        }

        process_pr_events(pr);
    }
}

void
assert_br(pr_state_t *pr)
{
    if (pr->bbsy_asserted) {
        return;
    }

    int ret;
    bus_req_t req;
    req.sig = BR;
    req.assertion = ASSERTED;
    memset(req.from, 0, sizeof(req.from));

    strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));

    ret = write(pr->pr_bus_out_l, &req, sizeof(req));
    if (ret == -1) {
        perror("assert_br_write_l");
        return;
    }

    ret = write(pr->pr_bus_out_r, &req, sizeof(req));
    if (ret == -1) {
        perror("assert_br_write_r");
        return;
    }

    pr->br_issued = TRUE;
}

void
assert_bbsy(pr_state_t *pr)
{
    if (pr->bbsy_asserted) {
        return;
    }

    int ret;
    bus_req_t req;
    req.sig = BBSY;
    req.assertion = ASSERTED;
    memset(req.from, 0, sizeof(req.from));

    strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));

    ret = write(pr->pr_bus_out_l, &req, sizeof(req));
    if (ret == -1) {
        perror("assert_bbsy_write_l");
        return;
    }

    ret = write(pr->pr_bus_out_r, &req, sizeof(req));
    if (ret == -1) {
        perror("assert_bbsy_write_r");
        return;
    }

    pr->bbsy_asserted = TRUE;

    pthread_mutex_lock(&(STATE->state_mutex));
    STATE->is_master = TRUE;
    STATE->need_master = FALSE;
    pthread_mutex_unlock(&(STATE->state_mutex));
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
    if (pr->pr_bus_out_l == -1 && pr->pr_bus_out_r == -1) {
        return -1;
    } else if (pr->pr_bus_out_l == -1) {
        if (reply) {
            return pr->pr_bus_out_r;
        } else {
            return -1;
        }
    } else if (pr->pr_bus_out_r == -1) {
        if (reply) {
            return pr->pr_bus_out_l;
        } else {
            return -1;
        }
    } else {
        if (strncmp(req->from, pr->pr_out_addr_l.sun_path, SOCK_NAME_LEN) == 0) {
            return reply ? pr->pr_bus_out_l : pr->pr_bus_out_r;
        } else {
            return reply ? pr->pr_bus_out_r : pr->pr_bus_out_l;
        }
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

    if (out_fd == -1) {
        return;
    }

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
    if (pr->br_issued && (req->assertion == ASSERTED)) {
        bus_req_t resp;
        resp.sig = SACK;
        resp.assertion = ASSERTED;
        update_from_addr(pr, &resp);

        out_fd = get_fd_direction(pr, req, TRUE);
        if (out_fd == -1) {
            return;
        }

        ret = write(out_fd, &resp, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_bg_block_write");
            return;
        }

        // Revert br_issued, as we're no longer waiting for a BG.
        pr->br_issued = FALSE;
        pr->sack_asserted = TRUE;
    } else if (pr->sack_asserted && (req->assertion == NEGATED)) {
        assert_bbsy(pr);

    // Else PASS the grant.
    } else {
        out_fd = get_fd_direction(pr, req, FALSE);
        if (out_fd == -1) {
            return;
        }

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

    if (out_fd != -1) {
        return;
    }

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
        if (out_fd == -1) {
            return;
        }

        ret = write(out_fd, &resp, sizeof(bus_req_t));
        if (ret == -1) {
            perror("handle_npg_block_write");
            return;
        }

        // Revert npr_issued, as we're no longer waiting for a NPG.
        pr->npr_issued = FALSE;
        pr->sack_asserted = TRUE;
    } else {
        // Pass the grant on.
        out_fd = get_fd_direction(pr, req, FALSE);
        if (out_fd == -1) {
            return;
        }

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
    if (out_fd == -1) {
        return;
    }

    update_from_addr(pr, req);
    ret = write(out_fd, req, sizeof(bus_req_t));
    if (ret == -1) {
        perror("handle_sack_write");
        return;
    }
}

void
handle_bbsy(pr_state_t *pr, bus_req_t *req)
{
    int ret;
    int out_fd;

    out_fd = get_fd_direction(pr, req, FALSE);

    if (req->assertion == ASSERTED) {
        pr->bbsy_asserted = TRUE;
    } else {
        pr->bbsy_asserted = FALSE;
    }

    if (out_fd == -1) {
        return;
    }

    update_from_addr(pr, req);
    ret = write(out_fd, req, sizeof(bus_req_t));
    if (ret == -1) {
        perror("handle_bbsy_write");
        return;
    }
}

