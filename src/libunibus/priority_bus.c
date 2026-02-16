#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bus_state.h"
#include "debug.h"
#include "priority_bus.h"
#include "signals.h"
#include "util.h"

/* State for the priority bus processor. */
typedef struct _pr_state {
	struct sockaddr_un pr_out_addr_l;
	struct sockaddr_un pr_out_addr_r;
	struct sockaddr_un pr_in_addr;

	int pr_bus_in;
	int pr_bus_out_l;
	int pr_bus_out_r;

	uint8_t npr_issued;
	uint8_t br_issued;
	uint8_t sack_asserted;
	uint8_t bbsy_asserted;
} pr_state;

static bus_state *STATE = NULL;

static direction get_fd_direction(pr_state*, pr_bus_req_t*, uint8_t);
static int get_fd(pr_state*, direction);
static void update_from_addr(pr_state*, pr_bus_req_t*);

void assert_br(pr_state*);
void assert_bbsy(pr_state*);
void negate_sack(pr_state*);
void negate_bbsy(pr_state*);

void process_pr_events(pr_state *pr);

void handle_br(pr_state*, pr_bus_req_t*);
void handle_bg(pr_state*, pr_bus_req_t*);
void handle_npr(pr_state*, pr_bus_req_t*);
void handle_npg(pr_state*, pr_bus_req_t*);
void handle_sack(pr_state*, pr_bus_req_t*);
void handle_bbsy(pr_state*, pr_bus_req_t*);

void request_master(pr_state*);
void release_master(pr_state*);

int pr_connect(pr_state*, direction);
int pr_connect_l(pr_state*);
int pr_connect_r(pr_state*);

int pr_close(pr_state*, direction);
int pr_close_l(pr_state*);
int pr_close_r(pr_state*);

int
pr_cleanup(pr_state *pr)
{
	close(pr->pr_bus_in);
	unlink(pr->pr_in_addr.sun_path);
	free(pr);

	/* FIXME */
	return 0;
}

pr_state*
init_pr_state(char *l_sock, char *sock, char *r_sock)
{
	int ret;

	pr_state *pr = malloc(sizeof(pr_state));
	if (pr == NULL) {
		perror("pr_malloc");
		return NULL;
	}

	if (l_sock != NULL) {
		memset(&(pr->pr_out_addr_l), 0, sizeof(struct sockaddr_un));
		pr->pr_out_addr_l.sun_family = AF_UNIX;
		//strncpy(
		//	pr->pr_out_addr_l.sun_path,
		//	l_sock,
		//	sizeof(pr->pr_out_addr_l.sun_path) - 1
		//);
		sprintf(pr->pr_out_addr_l.sun_path, "/tmp/xmachine/%s_pr.socket", l_sock);
	}

	if (r_sock != NULL) {
		memset(&(pr->pr_out_addr_r), 0, sizeof(struct sockaddr_un));
		pr->pr_out_addr_r.sun_family = AF_UNIX;
		//strncpy(
		//	pr->pr_out_addr_r.sun_path,
		//	r_sock,
		//	sizeof(pr->pr_out_addr_r.sun_path) - 1
		//);
		sprintf(pr->pr_out_addr_r.sun_path, "/tmp/xmachine/%s_pr.socket", r_sock);
	}

	memset(&(pr->pr_in_addr), 0, sizeof(struct sockaddr_un));
	//strncpy(pr->pr_in_addr.sun_path, sock, sizeof(pr->pr_in_addr.sun_path));
	sprintf(pr->pr_in_addr.sun_path, "/tmp/xmachine/%s_pr.socket", sock);
	pr->pr_in_addr.sun_family = AF_UNIX;

	pr->pr_bus_in = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (pr->pr_bus_in == -1) {
		perror("pr_bus_in");
		free(pr);
		return NULL;
	}

	if (l_sock == NULL) {
		pr->pr_bus_out_l = -1;
	}

	if (r_sock == NULL) {
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

	pr->npr_issued = 0;
	pr->br_issued = 0;
	pr->bbsy_asserted = 0;
	pr->sack_asserted = 0;

	return pr;
}

void*
priority_bus_mgr(void *bus)
{
	struct timespec wait;
	wait.tv_sec = 0;
	wait.tv_nsec = 300000000; /* 300ms */

	uint8_t is_master;
	uint8_t req_master;
	uint8_t rel_master;
	uint8_t shutdown;

	char l_sock_buf[100];
	char sock[100];
	char r_sock_buf[100];
	char *l_sock;
	char *r_sock;

	if (bus == NULL) {
		return NULL;
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

	pr_state *pr = init_pr_state(l_sock, sock, r_sock);
	if (pr == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&(STATE->pr_ready_mutex));
	pthread_cond_signal(&(STATE->cond_pr_ready));
	pthread_mutex_unlock(&(STATE->pr_ready_mutex));

	/* TODO Break this out into a function. */
	do {
		pthread_mutex_lock(&(STATE->state_mutex));
		is_master = STATE->is_master;
		req_master = STATE->req_master;
		rel_master = STATE->rel_master;
		shutdown = STATE->shutdown;
		pthread_mutex_unlock(&(STATE->state_mutex));

		if (req_master && (is_master == 0)) {
			request_master(pr);
		} else if (rel_master && (is_master == 1)) {
			release_master(pr);
		}

		process_pr_events(pr);

		nanosleep(&wait, NULL);
	} while (!shutdown);

	pr_cleanup(pr);

	return NULL;
}

void
assert_br(pr_state *pr)
{
	if (pr->bbsy_asserted && pr->br_issued == 0) {
		return;
	}

	int ret;

	pr_bus_req_t req;
	req.sig = BR;
	req.assertion = ASSERTED;
	memset(req.from, 0, sizeof(req.from));

	strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));
	ret = pr_connect_l(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_l, &req, sizeof(req));
		if (ret == -1) {
			perror("assert_br_write_l");
			return;
		}
		pr_close_l(pr);
	}

	ret = pr_connect_r(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_r, &req, sizeof(req));
		if (ret == -1) {
			perror("assert_br_write_r");
			return;
		}
		pr_close_r(pr);
	}

	pr->br_issued = 1;
}

void
assert_bbsy(pr_state *pr)
{
	if (pr->bbsy_asserted) {
		return;
	}

	int ret;
	pr_bus_req_t req;
	req.sig = BBSY;
	req.assertion = ASSERTED;
	memset(req.from, 0, sizeof(req.from));

	strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));

	ret = pr_connect_l(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_l, &req, sizeof(req));
		if (ret == -1) {
			perror("assert_bbsy_write_l");
			return;
		}
		pr_close_l(pr);
	}

	ret = pr_connect_r(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_r, &req, sizeof(req));
		if (ret == -1) {
			perror("assert_bbsy_write_r");
			return;
		}
		pr_close_r(pr);
	}

	pr->bbsy_asserted = 1;

	pthread_mutex_lock(&(STATE->state_mutex));
	STATE->is_master = 1;
	STATE->req_master = 0;
	pthread_mutex_unlock(&(STATE->state_mutex));
}

void
negate_sack(pr_state *pr)
{
	if (!pr->sack_asserted) {
		return;
	}

	int ret;
	pr_bus_req_t req;
	req.sig = SACK;
	req.assertion = NEGATED;
	memset(req.from, 0, sizeof(req.from));

	strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));

	ret = pr_connect_l(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_l, &req, sizeof(req));
		if (ret == -1) {
			perror("negate_sack_write_l");
			return;
		}
		pr_close_l(pr);
	}

	ret = pr_connect_r(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_r, &req, sizeof(req));
		if (ret == -1) {
			perror("negate_sack_write_r");
			return;
		}
		pr_close_r(pr);
	}

	pr->sack_asserted = 0;

	pthread_mutex_lock(&(STATE->state_mutex));
	STATE->is_master = 0;
	STATE->req_master = 0;
	pthread_mutex_unlock(&(STATE->state_mutex));
}

void
negate_bbsy(pr_state *pr)
{
	if (!pr->bbsy_asserted) {
		return;
	}

	int ret;
	pr_bus_req_t req;
	req.sig = BBSY;
	req.assertion = NEGATED;
	memset(req.from, 0, sizeof(req.from));

	strncpy(req.from, pr->pr_in_addr.sun_path, sizeof(req.from));

	ret = pr_connect_l(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_l, &req, sizeof(req));
		if (ret == -1) {
			perror("negate_bbsy_write_l");
			return;
		}
		pr_close_l(pr);
	}

	ret = pr_connect_r(pr);
	if (ret != -1) {
		ret = write(pr->pr_bus_out_r, &req, sizeof(req));
		if (ret == -1) {
			perror("negate_bbsy_write_r");
			return;
		}
		pr_close_r(pr);
	}

	pr->bbsy_asserted = 0;

	pthread_mutex_lock(&(STATE->state_mutex));
	STATE->is_master = 0;
	STATE->req_master = 0;
	STATE->rel_master = 0;
	pthread_mutex_unlock(&(STATE->state_mutex));
}

/*
 * Process a single cycle on the Unibus priority bus (one read and one write).
 */
void
process_pr_events(pr_state *pr)
{
	int ret;
	int err;
	pr_bus_req_t event;

	ret = recv(pr->pr_bus_in, &event, sizeof(pr_bus_req_t), MSG_DONTWAIT);
	if (ret == -1) {
		err = errno;
		if (err == EAGAIN || err ==  EWOULDBLOCK) {
			return;
		} else {
			perror("pr_event_read");
			return;
		}
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
get_fd(pr_state *pr, direction dir)
{
	switch (dir) {
	case D_LEFT:
		return pr->pr_bus_out_l;
	case D_RIGHT:
		return pr->pr_bus_out_r;
	case D_NONE:
		return -1;
	}

	return -1;
}

direction
get_fd_direction(pr_state *pr, pr_bus_req_t *req, uint8_t reply)
{
	if (pr->pr_bus_out_l == -1 && pr->pr_bus_out_r == -1) {
		return D_NONE;
	} else if (pr->pr_bus_out_l == -1) {
		if (reply) {
			return D_RIGHT;
		} else {
			return D_NONE;
		}
	} else if (pr->pr_bus_out_r == -1) {
		if (reply) {
			return D_LEFT;
		} else {
			return D_NONE;
		}
	} else {
		if (strncmp(req->from, pr->pr_out_addr_l.sun_path, SOCK_NAME_LEN) == 0) {
			return reply ? D_LEFT : D_RIGHT;
		} else {
			return reply ? D_RIGHT : D_LEFT;
		}
	}
}

void
update_from_addr(pr_state *pr, pr_bus_req_t *req)
{
	memset(req->from, 0, sizeof(req->from));
	strncpy(req->from, pr->pr_in_addr.sun_path, sizeof(req->from));
}

void
handle_br(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;

	dir = get_fd_direction(pr, req, 0);
	if (dir == D_NONE) {
		return;
	}

	out_fd = get_fd(pr, dir);

	update_from_addr(pr, req);
	pr_connect(pr, dir);
	ret = write(out_fd, req, sizeof(pr_bus_req_t));
	if (ret == -1) {
		perror("handle_br_write");
		return;
	}
	pr_close(pr, dir);
}

void
handle_bg(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;

	/*
	 * If we issued a bus request:
	 * - BLOCK the grant.
	 * - Assert a SACK.
	 */
	if (pr->br_issued && (req->assertion == ASSERTED)) {
		pr_bus_req_t resp;
		resp.sig = SACK;
		resp.assertion = ASSERTED;
		update_from_addr(pr, &resp);

		dir = get_fd_direction(pr, req, 1);
		if (dir == D_NONE) {
			return;
		}

		out_fd = get_fd(pr, dir);

		ret = pr_connect(pr, dir);
		if (ret != 0) {
			dbg_bus(STATE, "pr_connect failed");
		}
		fprintf(stderr, "%s: %s: dir %s\n", "handle_bg", STATE->sock, DIRC(dir));

		ret = write(out_fd, &resp, sizeof(pr_bus_req_t));
		if (ret == -1) {
			perror("handle_bg_block_write");
			return;
		}
		pr_close(pr, dir);

		/* Revert br_issued, as we're no longer waiting for a BG. */
		pr->br_issued = 0;
		pr->sack_asserted = 1;
	} else if (pr->sack_asserted && (req->assertion == NEGATED)) {
		assert_bbsy(pr);

		/* Unblock the main thread waiting on bus privs. */
		pthread_mutex_lock(&(STATE->pr_master_mutex));
		pthread_cond_signal(&(STATE->cond_pr_master));
		pthread_mutex_unlock(&(STATE->pr_master_mutex));
		dbg_bus(STATE, "handle_bg: asserted BBSY, unblocked main");

	/* Else PASS the grant. */
	} else {
		dir = get_fd_direction(pr, req, 0);
		if (dir == D_NONE) {
			return;
		}

		out_fd = get_fd(pr, dir);

		update_from_addr(pr, req);
		pr_connect(pr, dir);
		ret = write(out_fd, req, sizeof(pr_bus_req_t));
		if (ret == -1) {
			perror("handle_bg_pass_write");
			return;
		}
		pr_close(pr, dir);
	}
}

void
handle_npr(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;
	dir = get_fd_direction(pr, req, 0);

	if (dir == D_NONE) {
		return;
	}

	out_fd = get_fd(pr, dir);

	update_from_addr(pr, req);
	pr_connect(pr, dir);
	ret = write(out_fd, req, sizeof(pr_bus_req_t));
	if (ret == -1) {
		perror("handle_npr_write");
		return;
	}
	pr_close(pr, dir);
}

void
handle_npg(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;

	/* Second verse, same as the first. */
	/* Block grant and SACK. */
	if (pr->npr_issued) {
		pr_bus_req_t resp;
		resp.sig = SACK;
		resp.assertion = ASSERTED;
		update_from_addr(pr, &resp);

		dir = get_fd_direction(pr, req, 1);
		if (dir == D_NONE) {
			return;
		}

		out_fd = get_fd(pr, dir);

		pr_connect(pr, dir);
		ret = write(out_fd, &resp, sizeof(pr_bus_req_t));
		if (ret == -1) {
			perror("handle_npg_block_write");
			return;
		}
		pr_close(pr, dir);

		/* Revert npr_issued, as we're no longer waiting for a NPG. */
		pr->npr_issued = 0;
		pr->sack_asserted = 1;
	} else {
		/* Pass the grant on. */
		dir = get_fd_direction(pr, req, 0);
		if (dir == D_NONE) {
			return;
		}

		out_fd = get_fd(pr, dir);

		update_from_addr(pr, req);
		pr_connect(pr, dir);
		ret = write(out_fd, req, sizeof(pr_bus_req_t));
		if (ret == -1) {
			perror("handle_npg_pass_write");
			return;
		}
		pr_close(pr, dir);
	}
}

void
handle_sack(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;

	dir = get_fd_direction(pr, req, 0);
	if (dir == D_NONE) {
		return;
	}

	out_fd = get_fd(pr, dir);

	update_from_addr(pr, req);
	pr_connect(pr, dir);
	ret = write(out_fd, req, sizeof(pr_bus_req_t));
	if (ret == -1) {
		perror("handle_sack_write");
		return;
	}
	pr_close(pr, dir);
}

void
handle_bbsy(pr_state *pr, pr_bus_req_t *req)
{
	int ret;
	int out_fd;
	direction dir;

	dir = get_fd_direction(pr, req, 0);

	if (req->assertion == ASSERTED) {
		pr->bbsy_asserted = 1;
	} else {
		pr->bbsy_asserted = 0;
	}

	if (dir == D_NONE) {
		return;
	}

	out_fd = get_fd(pr, dir);

	update_from_addr(pr, req);
	pr_connect(pr, dir);
	ret = write(out_fd, req, sizeof(pr_bus_req_t));
	if (ret == -1) {
		perror("handle_bbsy_write");
		return;
	}
	pr_close(pr, dir);
}

void
request_master(pr_state *pr)
{
	if (pr->bbsy_asserted) {
		return;
	}

	assert_br(pr);

	pthread_mutex_lock(&(STATE->state_mutex));
	STATE->req_master = 0;
	pthread_mutex_unlock(&(STATE->state_mutex));
}

void
release_master(pr_state *pr)
{
	negate_sack(pr);
	negate_bbsy(pr);

	pthread_mutex_lock(&(STATE->state_mutex));
	printf("%s: release_master: current state:\n", STATE->sock);
	printf("%s: release_master: is_master: %d\n", STATE->sock, STATE->is_master);
	printf("%s: release_master: rel_master: %d\n", STATE->sock, STATE->rel_master);
	printf("%s: release_master: req_master: %d\n", STATE->sock, STATE->req_master);
	printf("%s: release_master: npr_issued: %d\n", STATE->sock, pr->npr_issued);
	printf("%s: release_master: br_issued: %d\n", STATE->sock, pr->br_issued);
	printf("%s: release_master: sack_asserted: %d\n", STATE->sock, pr->sack_asserted);
	printf("%s: release_master: bbsy_asserted: %d\n", STATE->sock, pr->bbsy_asserted);
	pthread_mutex_unlock(&(STATE->state_mutex));

	/* Signal that we are no longer master. */
	pthread_mutex_lock(&(STATE->pr_rel_master_mutex));
	pthread_cond_signal(&(STATE->cond_pr_rel_master));
	pthread_mutex_unlock(&(STATE->pr_rel_master_mutex));
}

int
pr_connect(pr_state *pr, direction dir)
{
	switch (dir) {
	case D_LEFT:
		return pr_connect_l(pr);
	case D_RIGHT:
		return pr_connect_r(pr);
	case D_NONE:
		return -1;
	}

	return -1;
}

int
pr_connect_l(pr_state *pr)
{
	int ret;
	if (pr->pr_bus_out_l == -1) {
		return -1;
	}

	pr->pr_bus_out_l = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (pr->pr_bus_out_l == -1) {
		perror("pr_connect_l-socket");
		ret = -1;
		return ret;
	}

	ret = connect(
		pr->pr_bus_out_l,
		(const struct sockaddr *) &(pr->pr_out_addr_l),
		sizeof(pr->pr_out_addr_l)
	);
	if (ret == -1) {
		perror("pr_connect_l-connect");
		return ret;
	}

	return 0;
}

int
pr_connect_r(pr_state *pr)
{
	int ret;
	if (pr->pr_bus_out_r == -1) {
		return -1;
	}

	pr->pr_bus_out_r = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (pr->pr_bus_out_r == -1) {
		perror("pr_connect_r-socket");
		ret = -1;
		return ret;
	}

	ret = connect(
		pr->pr_bus_out_r,
		(const struct sockaddr *) &(pr->pr_out_addr_r),
		sizeof(pr->pr_out_addr_r)
	);
	if (ret == -1) {
		perror("pr_connect_r-connect");
		printf("r_sock: %s\n", pr->pr_out_addr_r.sun_path);
		return ret;
	}

	return 0;
}

int
pr_close(pr_state *pr, direction dir)
{
	switch (dir) {
	case D_LEFT:
		return pr_close_l(pr);
	case D_RIGHT:
		return pr_close_r(pr);
	case D_NONE:
		return -1;
	}

	return -1;
}

int
pr_close_l(pr_state *pr)
{
	int ret;
	ret = close(pr->pr_bus_out_l);
	if (ret == -1) {
		perror("pr_close_l-close");
	}

	return ret;
}

int
pr_close_r(pr_state *pr)
{
	int ret;
	ret = close(pr->pr_bus_out_r);
	if (ret == -1) {
		perror("pr_close_r-close");
	}

	return ret;
}

