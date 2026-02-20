#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bus_state.h"
#include "debug.h"
#include "signals.h"
#include "util.h"

/* State for the data bus processor. */
typedef struct _data_state {
	struct sockaddr_un d_out_addr_l;
	struct sockaddr_un d_out_addr_r;
	struct sockaddr_un d_in_addr;

	int d_bus_in;
	int d_bus_out_l;
	int d_bus_out_r;

	data_xfer_spec buffer;

	uint8_t req_issued;
	uint8_t ack_received;

	uint8_t (*is_addr_internal)(uint32_t);
} data_state;

typedef enum _data_bus_msg {
	DBM_REQ,
	DBM_RESP
} data_bus_msg;

/* I'm lazy, so the control lines (C0, C1) are just mnemonics now. */
typedef enum _data_bus_signal {
	D_DATI,
	D_DATIP,
	D_DATO,
	D_DATOB,
	D_EMPTY
} data_bus_signal;

typedef struct _data_bus_req {
	char from[SOCK_NAME_LEN];
	data_bus_msg msg_type;

	data_bus_signal c;
	uint32_t addr;
	uint16_t data;

	assert_t msyn;
	assert_t ssyn;
} data_bus_req;


static bus_state *STATE = NULL;
static data_state *DATA_BUS_STATE = NULL;

static void update_from_addr(data_bus_req*);

static data_state* init_data_state(char*, char*, char*);

static void cleanup();

extern void* data_bus_mgr(void*);

static void in_word();
static void out_word();
static void out_byte();
static void process_op();
static void check_bus(uint8_t);
static void handle_resp(data_bus_req*);
static uint8_t examine_address(data_bus_req*);
static void handle_req(data_bus_req*);

static void handle_dati(data_bus_req*);
/* static void handle_datip(uint32_t);*/
static void handle_dato(data_bus_req*);
static void handle_datob(data_bus_req*);
static void slave_wait();

static int d_connect_l();
static int d_connect_r();

static int d_close_l();
static int d_close_r();

static int send_msg(data_bus_req*);

static void
update_from_addr(data_bus_req *req)
{
	memset(req->from, 0, sizeof(req->from));
	strncpy(req->from, DATA_BUS_STATE->d_in_addr.sun_path, sizeof(req->from));
}

data_state*
init_data_state(char *l_sock, char *sock, char *r_sock)
{
	int ret;

	data_state *d = malloc(sizeof(data_state));
	if (d == NULL) {
		perror("init_data");
		return NULL;
	}

	d->req_issued = 0;
	d->ack_received = 0;

	d->buffer.op = R_NONE;
	d->buffer.addr = 0;
	d->buffer.value = 0;

	memset(&(d->d_out_addr_l), 0, sizeof(struct sockaddr_un));
	if (l_sock != NULL) {
		d->d_out_addr_l.sun_family = AF_UNIX;
		sprintf(d->d_out_addr_l.sun_path, "/tmp/xmachine/%s_d.socket", l_sock);
	}

	memset(&(d->d_out_addr_r), 0, sizeof(struct sockaddr_un));
	if (r_sock != NULL) {
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
cleanup()
{
	if (DATA_BUS_STATE != NULL) {
		close(DATA_BUS_STATE->d_bus_in);
		unlink(DATA_BUS_STATE->d_in_addr.sun_path);
		free(DATA_BUS_STATE);
	}
}

void*
data_bus_mgr(void *bus)
{
	struct timespec wait;
	wait.tv_sec = 0;
	wait.tv_nsec = 300000000; /* 300ms */

	uint8_t is_master;
	uint8_t shutdown;
	data_op master_op;

	char l_sock_buf[100];
	char sock[100];
	char r_sock_buf[100];
	char *l_sock;
	char *r_sock;

	uint8_t (*addr_ptr)(uint32_t) = NULL;

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

	DATA_BUS_STATE = init_data_state(l_sock, sock, r_sock);
	if (DATA_BUS_STATE == NULL) {
		return NULL;
	}

	DATA_BUS_STATE->is_addr_internal = addr_ptr;

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
				DATA_BUS_STATE->buffer.op = R_IN;
				DATA_BUS_STATE->buffer.addr = STATE->master.addr;
				STATE->master.op = R_NONE;
				break;
			case R_INB:
				DATA_BUS_STATE->buffer.op = R_INB;
				DATA_BUS_STATE->buffer.addr = STATE->master.addr;
				STATE->master.op = R_NONE;
				break;
			case R_OUT:
				DATA_BUS_STATE->buffer.op = R_OUT;
				DATA_BUS_STATE->buffer.addr = STATE->master.addr;
				DATA_BUS_STATE->buffer.value = STATE->master.value;
				STATE->master.op = R_NONE;
				break;
			case R_OUTB:
				DATA_BUS_STATE->buffer.op = R_OUTB;
				DATA_BUS_STATE->buffer.addr = STATE->master.addr;
				DATA_BUS_STATE->buffer.value = STATE->master.value;
				STATE->master.op = R_NONE;
				break;

			/* Fallthrough cases that do not apply. */
			case R_BLOCK_IN:
			case R_BLOCK_INB:
			case R_BLOCK_OUT:
			case R_BLOCK_OUTB:
			case R_DONE:
			case R_NONE:
				STATE->master.op = R_NONE;
			}
		}
		pthread_mutex_unlock(&(STATE->state_mutex));

		if (is_master) {
			process_op();
		}
		check_bus(is_master);
		nanosleep(&wait, NULL);
	} while (!shutdown);

	cleanup();

	return NULL;
}

void
in_word()
{
	dbg_bus(STATE, "data_bus:in_word");
	int ret;
	data_bus_req req;
	req.msg_type = DBM_REQ;
	req.c = D_DATI;
	req.addr = DATA_BUS_STATE->buffer.addr;

	update_from_addr(&req);
	ret = send_msg(&req);
	if (ret != 0) {
		/* Goodbye forever. */
		fprintf(stderr, "DATI: failed to send data bus message\n");
		fprintf(stderr, "DATI: this means the main thread will block forever\n");
		fprintf(stderr, "DATI: goodbye\n");
	}
}

void
out_word()
{
	/*
	 * FIXME See in_word for details on just returning below.
	 */

	int ret;
	data_bus_req req;
	req.msg_type = DBM_REQ;
	req.c = D_DATO;
	req.addr = DATA_BUS_STATE->buffer.addr;
	req.data = DATA_BUS_STATE->buffer.value;

	update_from_addr(&req);
	ret = send_msg(&req);
	if (ret != 0) {
		fprintf(stderr, "DATO: failed to send data bus message\n");
		fprintf(stderr, "DATO: blocking forever, goodbye.\n");
		return;
	}
}

void
out_byte()
{
	/*
	 * FIXME See in_word for details on just returning below.
	 */
	dbg_bus(STATE, "data_bus:out_byte");

	int ret;
	data_bus_req req;
	req.msg_type = DBM_REQ;
	req.c = D_DATOB;
	req.addr = DATA_BUS_STATE->buffer.addr;
	req.data = DATA_BUS_STATE->buffer.value;

	update_from_addr(&req);
	ret = send_msg(&req);
	if (ret != 0) {
		fprintf(stderr, "DATOB: failed to send data bus message\n");
		fprintf(stderr, "DATOB: blocking forever, goodbye.\n");
		return;
	}
}

void
process_op()
{
		switch (DATA_BUS_STATE->buffer.op) {
		case R_IN:
			in_word();
			DATA_BUS_STATE->req_issued = 1;
			break;
		case R_OUT:
			out_word();
			DATA_BUS_STATE->req_issued = 1;
			break;
		case R_OUTB:
			out_byte();
			DATA_BUS_STATE->req_issued = 1;
			break;

		/* Fallthrough cases that do not apply. */
		case R_INB:
		case R_BLOCK_IN:
		case R_BLOCK_INB:
		case R_BLOCK_OUT:
		case R_BLOCK_OUTB:
		case R_DONE:
		case R_NONE:
		default:
			break;
		}
		DATA_BUS_STATE->buffer.op = R_NONE;
}

void
check_bus(uint8_t is_master)
{
	int ret;
	int err;
	uint8_t is_this_device = 0;
	data_bus_req event;

	ret = recv(DATA_BUS_STATE->d_bus_in, &event, sizeof(data_bus_req), MSG_DONTWAIT);
	if (ret == -1) {
		err = errno;
		if (err == EAGAIN || err ==  EWOULDBLOCK) {
			return;
		} else {
			perror("check_bus-recv");
			return;
		}
	}

	is_this_device = examine_address(&event);
	if (event.msg_type == DBM_REQ && is_this_device) {
		handle_req(&event);
	} else if (
		event.msg_type == DBM_RESP &&
		is_master &&
		DATA_BUS_STATE->req_issued
	) {
		handle_resp(&event);
	}
}

void
handle_resp(data_bus_req *event)
{
	dbg_bus(STATE, "handle_resp");
	switch (event->c) {
	case D_DATI: /* Fallthrough, all transactions are handled the same. */
	case D_DATO:
	case D_DATOB:
		pthread_mutex_lock(&(STATE->master_xfer_mutex));
		STATE->master.value = event->data;
		STATE->master.op = R_DONE;
		pthread_mutex_unlock(&(STATE->master_xfer_mutex));
		break;
	default:
		return;
	}

	pthread_mutex_lock(&(STATE->master_data_mutex));
	pthread_cond_signal(&(STATE->cond_master_data));
	pthread_mutex_unlock(&(STATE->master_data_mutex));

	DATA_BUS_STATE->req_issued = 0;
}

uint8_t
examine_address(data_bus_req *event)
{
	if (DATA_BUS_STATE->is_addr_internal == NULL) {
		return 0;
	}

	return DATA_BUS_STATE->is_addr_internal(event->addr);
}

void
handle_req(data_bus_req *event)
{
	switch (event->c) {
	case D_DATI:
		handle_dati(event);
		return;
	case D_DATIP:
		return;
	case D_DATO:
		handle_dato(event);
		return;
	case D_DATOB:
		handle_datob(event);
		return;
	case D_EMPTY:
		return;
	}
}

void
handle_dati(data_bus_req *event)
{
	dbg_bus(STATE, "handle_dati");

	int ret;
	data_bus_req resp;
	resp.msg_type = DBM_RESP;
	resp.c = D_DATI;

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
	STATE->slave.op = R_BLOCK_IN;
	STATE->slave.addr = event->addr;
	STATE->slave.value = 0;
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	slave_wait();

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
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
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	ret = send_msg(&resp);
	if (ret != 0) {
		dbg_bus(STATE, "handle_dati: failed to send message");
	}
}

void
handle_dato(data_bus_req *event)
{
	dbg_bus(STATE, "handle_dato");

	int ret;
	data_bus_req resp;
	resp.msg_type = DBM_RESP;
	resp.c = D_DATO;

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
	STATE->slave.op = R_BLOCK_OUT;
	STATE->slave.addr = event->addr;
	STATE->slave.value = event->data;
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	slave_wait();

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
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
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	ret = send_msg(&resp);
	if (ret != 0) {
		dbg_bus(STATE, "handle_dati: failed to send message");
	}
}

void
handle_datob(data_bus_req *event)
{
	dbg_bus(STATE, "handle_datob");

	int ret;
	data_bus_req resp;
	resp.msg_type = DBM_RESP;
	resp.c = D_DATOB;

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
	STATE->slave.op = R_BLOCK_OUTB;
	STATE->slave.addr = event->addr;
	STATE->slave.value = event->data;
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	slave_wait();

	pthread_mutex_lock(&(STATE->slave_xfer_mutex));
	if (STATE->slave.op == R_DONE) {
		resp.addr = 0;
		resp.data = 0;
		resp.ssyn = ASSERTED;

		STATE->slave.op = R_NONE;
		STATE->slave.addr = 0;
		STATE->slave.value = 0;
	} else {
		fprintf(stderr, "slave operation not completed. this is bad.\n");
		resp.ssyn = NEGATED;
	}
	pthread_mutex_unlock(&(STATE->slave_xfer_mutex));

	ret = send_msg(&resp);
	if (ret != 0) {
		dbg_bus(STATE, "handle_dati: failed to send message");
	}
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
d_connect_l()
{
	int ret;
	if (DATA_BUS_STATE->d_out_addr_l.sun_path[0] == 0) {
		return -1;
	}

	ret = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (ret == -1) {
		perror("d_connect_l-socket");
		return ret;
	}

	DATA_BUS_STATE->d_bus_out_l = ret;

	ret = connect(
		DATA_BUS_STATE->d_bus_out_l,
		(const struct sockaddr *) &(DATA_BUS_STATE->d_out_addr_l),
		sizeof(DATA_BUS_STATE->d_out_addr_l)
	);
	if (ret == -1) {
		perror("d_connect_l-connect");
		return ret;
	}

	return 0;
}

int
d_connect_r()
{
	int ret;
	if (DATA_BUS_STATE->d_out_addr_r.sun_path[0] == 0) {
		return -1;
	}

	DATA_BUS_STATE->d_bus_out_r = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (DATA_BUS_STATE->d_bus_out_r == -1) {
		perror("d_connect_r-socket");
		ret = -1;
		return ret;
	}

	ret = connect(
		DATA_BUS_STATE->d_bus_out_r,
		(const struct sockaddr *) &(DATA_BUS_STATE->d_out_addr_r),
		sizeof(DATA_BUS_STATE->d_out_addr_r)
	);
	if (ret == -1) {
		perror("d_connect_r-connect");
		return ret;
	}

	return 0;
}

int
d_close_l()
{
	int ret;
	ret = close(DATA_BUS_STATE->d_bus_out_l);
	if (ret == -1) {
		perror("d_close_l-close");
	}

	DATA_BUS_STATE->d_bus_out_l = -1;

	return ret;
}

int
d_close_r()
{
	int ret;
	ret = close(DATA_BUS_STATE->d_bus_out_r);
	if (ret == -1) {
		perror("d_close_r-close");
	}

	DATA_BUS_STATE->d_bus_out_r = -1;

	return ret;
}

int
send_msg(data_bus_req *req)
{
	dbg_bus(STATE, "send_msg");

	int ret;
	ret = d_connect_l();
	if (ret != -1) {
		ret = write(DATA_BUS_STATE->d_bus_out_l, req, sizeof(data_bus_req));
		if (ret == -1) {
			perror("send_msg_l");
			return ret;
		}
		d_close_l();
	}

	ret = d_connect_r();
	if (ret != -1) {
		ret = write(DATA_BUS_STATE->d_bus_out_r, req, sizeof(data_bus_req));
		if (ret == -1) {
			perror("send_msg_r");
			return ret;
		}
		d_close_r();
	}

	return 0;
}

