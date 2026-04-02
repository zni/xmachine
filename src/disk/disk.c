#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "disk.h"
#include "../libunibus/device_bus_mgr.h"

static void init_disk();
static void shutdown_disk();
static void execute();
static void dump_disk_state();
static void pending_bus_requests();
static void fill_buffer();
static void empty_buffer();
static void read_sector();
static void write_sector();
static void clear_buffer_register();
static void set_status_register(uint16_t);
static void set_transfer_flag();
static void clear_transfer_flag();
static uint8_t is_transfer_flag_set();
static void set_done_flag();
/* static void clear_done_flag(); */
/* static uint8_t is_done_flag_set(); */
/* static void clear_writable_flags(); */
static void clear_all_flags();
static void usage();
static void handler(int, siginfo_t*, void*);

static disk_state DSK_STATE;
static bus_state *BUS_STATE = NULL;

void
init_disk()
{
	DSK_STATE.op = DSK_OP_BEGIN;
	DSK_STATE.func = DSK_FUNC_IDLE;
	DSK_STATE.track = 0;
	DSK_STATE.sector = 0;
	DSK_STATE.rxcs = 0;
	DSK_STATE.rxdb = 0;
	DSK_STATE.buffer_idx = 0;
	DSK_STATE.disk_media = -1;
}

void
shutdown_disk()
{
	if (DSK_STATE.disk_media == -1) {
		return;
	}

	close(DSK_STATE.disk_media);
}

uint8_t
is_local_addr(uint32_t addr)
{
	if (addr == RXCS_ADDR || addr == RXDB_ADDR) {
		/* process_bus_message(t, addr, data); */
		return 1;
	}

	return 0;
}

void
insert_disk_media(char *disk)
{
	int fd;
	fd = open(disk, O_RDWR);
	if (fd == -1) {
		perror("insert_disk_media-open");
		return;
	}

	DSK_STATE.disk_media = fd;
}

void
execute()
{
	uint16_t go_flag;
	uint8_t halt = 0;

	pthread_mutex_lock(&(BUS_STATE)->state_mutex);
	halt = BUS_STATE->shutdown;
	pthread_mutex_unlock(&(BUS_STATE)->state_mutex);
	while (!halt) {
		pending_bus_requests();

		go_flag = RXCS_FLAG_GO & DSK_STATE.rxcs;
		if (go_flag && ((DSK_STATE.op == DSK_OP_BEGIN) ||
		                (DSK_STATE.op == DSK_OP_DONE))) {
			int function = (RXCS_FLAG_FS & DSK_STATE.rxcs) >> 1;
			clear_all_flags();

			switch (function) {
				case DSK_FUNC_FILL_BUFFER:
					DSK_STATE.op = DSK_OP_FILL;
					DSK_STATE.func = DSK_FUNC_FILL_BUFFER;
					set_transfer_flag();
					break;
				case DSK_FUNC_EMPTY_BUFFER:
					DSK_STATE.op = DSK_OP_EMPTY;
					DSK_STATE.func = DSK_FUNC_EMPTY_BUFFER;
					break;
				case DSK_FUNC_WRITE_SECTOR:
					DSK_STATE.op = DSK_OP_SECTOR;
					DSK_STATE.func = DSK_FUNC_WRITE_SECTOR;
					set_transfer_flag();
					break;
				case DSK_FUNC_READ_SECTOR:
					DSK_STATE.op = DSK_OP_SECTOR;
					DSK_STATE.func = DSK_FUNC_READ_SECTOR;
					set_transfer_flag();
					break;
				case DSK_FUNC_NOT_USED:
					break;
				case DSK_FUNC_READ_STATUS:
					break;
				case DSK_FUNC_WRITE_DEL:
					break;
				case DSK_FUNC_READ_ERR:
					break;
				default:
					break;
			}

		} else if ((DSK_STATE.op == DSK_OP_FILL) &&
		           (DSK_STATE.func == DSK_FUNC_FILL_BUFFER)) {
			fill_buffer();
		} else if ((DSK_STATE.op == DSK_OP_EMPTY) &&
		           (DSK_STATE.func == DSK_FUNC_EMPTY_BUFFER)) {
			empty_buffer();
		} else if (DSK_STATE.func == DSK_FUNC_READ_SECTOR) {
			read_sector();
		} else if (DSK_STATE.func == DSK_FUNC_WRITE_SECTOR) {
			write_sector();
		} else if (DSK_STATE.op == DSK_OP_DONE) {
			DSK_STATE.op = DSK_OP_BEGIN;
			set_done_flag();
		}
		pthread_mutex_lock(&(BUS_STATE)->state_mutex);
		halt = BUS_STATE->shutdown;
		pthread_mutex_unlock(&(BUS_STATE)->state_mutex);
	}
}

void
dump_disk_state()
{
	fprintf(stdout, "RXCS: %07o\n", DSK_STATE.rxcs);
	fprintf(stdout, "RXDB: %07o\n", DSK_STATE.rxdb);
}

void
pending_bus_requests()
{
	data_xfer_spec slave_req;

	pthread_mutex_lock(&(BUS_STATE->perma_slave_mutex));
	pthread_cond_wait(&(BUS_STATE->cond_perma_slave), &(BUS_STATE->perma_slave_mutex));
	pthread_mutex_unlock(&(BUS_STATE->perma_slave_mutex));

	switch (slave_req.op) {
	case R_BLOCK_IN:
		if (slave_req.addr == RXCS_ADDR) {
			slave_req.value = DSK_STATE.rxcs;
			data_bus_reply(BUS_STATE, &slave_req);
		} else if (slave_req.addr == RXDB_ADDR) {
			slave_req.value = DSK_STATE.rxdb;
			clear_transfer_flag();
			clear_buffer_register();
			data_bus_reply(BUS_STATE, &slave_req);
		}
		break;

	case R_BLOCK_OUTB:
		if (slave_req.addr == RXCS_ADDR) {
			set_status_register(slave_req.value);
			data_bus_reply(BUS_STATE, &slave_req);
		} else if (slave_req.addr == RXDB_ADDR) {
			DSK_STATE.rxdb = slave_req.value;
			clear_transfer_flag();
			data_bus_reply(BUS_STATE, &slave_req);
		}
		break;

	case R_BLOCK_OUT:
		if (slave_req.addr == RXCS_ADDR) {
			set_status_register(slave_req.value);
			data_bus_reply(BUS_STATE, &slave_req);
		} else if (slave_req.addr == RXDB_ADDR) {
			DSK_STATE.rxdb = slave_req.value;
			clear_transfer_flag();
			data_bus_reply(BUS_STATE, &slave_req);
		}
		break;
	}
}

void
fill_buffer()
{
	if (DSK_STATE.buffer_idx == SECTOR_SIZE) {
		return;
	}

	if (is_transfer_flag_set()) {
		return;
	}

	clear_transfer_flag();

	DSK_STATE.internal_buffer[DSK_STATE.buffer_idx] = (
		DSK_STATE.rxdb & 0377
	);
	clear_buffer_register();
	DSK_STATE.buffer_idx++;
	if (DSK_STATE.buffer_idx != SECTOR_SIZE) {
		 set_transfer_flag();
	} else {
		clear_all_flags();
		set_done_flag();

		DSK_STATE.buffer_idx = 0;
		DSK_STATE.op = DSK_OP_DONE;
		DSK_STATE.func = DSK_FUNC_IDLE;
	}
}

void
empty_buffer()
{
	if (DSK_STATE.buffer_idx == SECTOR_SIZE) {
		return;
	}

	if (is_transfer_flag_set()) {
		return;
	}

	DSK_STATE.rxdb = (
		DSK_STATE.internal_buffer[DSK_STATE.buffer_idx]
	);
	DSK_STATE.buffer_idx++;
	if (DSK_STATE.buffer_idx != SECTOR_SIZE) {
		set_transfer_flag();
	} else {
		clear_all_flags();
		set_done_flag();

		DSK_STATE.buffer_idx = 0;
		DSK_STATE.op = DSK_OP_DONE;
		DSK_STATE.func = DSK_FUNC_IDLE;
	}
}

void
read_sector()
{
	if (DSK_STATE.op == DSK_OP_SECTOR && !is_transfer_flag_set()) {
		DSK_STATE.sector = DSK_STATE.rxdb;
		DSK_STATE.op = DSK_OP_TRACK;
		set_transfer_flag();
	} else if (DSK_STATE.op == DSK_OP_TRACK && !is_transfer_flag_set()) {
		DSK_STATE.track = DSK_STATE.rxdb;
		DSK_STATE.op = DSK_OP_WRITE_SECTOR;
	} else if (DSK_STATE.op == DSK_OP_WRITE_SECTOR) {
		if (DSK_STATE.disk_media != -1) {
			uint32_t track_offset = TRACK_SIZE * DSK_STATE.track;
			uint32_t disk_offset = track_offset + (SECTOR_SIZE * DSK_STATE.sector);
			lseek(DSK_STATE.disk_media, disk_offset, SEEK_SET);
			read(DSK_STATE.disk_media, DSK_STATE.internal_buffer, sizeof(uint8_t) * SECTOR_SIZE);
		}

		DSK_STATE.op = DSK_OP_DONE;
		DSK_STATE.func = DSK_FUNC_IDLE;
		DSK_STATE.sector = 0;
		DSK_STATE.track = 0;
	}
}

void
write_sector()
{
	if (DSK_STATE.op == DSK_OP_SECTOR && !is_transfer_flag_set()) {
		DSK_STATE.sector = DSK_STATE.rxdb;
		DSK_STATE.op = DSK_OP_TRACK;
		set_transfer_flag();
	} else if (DSK_STATE.op == DSK_OP_TRACK && !is_transfer_flag_set()) {
		DSK_STATE.track = DSK_STATE.rxdb;
		DSK_STATE.op = DSK_OP_WRITE_SECTOR;
	} else if (DSK_STATE.op == DSK_OP_WRITE_SECTOR) {
		if (DSK_STATE.disk_media != -1) {
			uint32_t track_offset = TRACK_SIZE * DSK_STATE.track;
			uint32_t disk_offset = track_offset + (SECTOR_SIZE * DSK_STATE.sector);
			lseek(DSK_STATE.disk_media, disk_offset, SEEK_SET);
			/* XXX was internal_buffer previously uint8_t? */
			write(DSK_STATE.disk_media, DSK_STATE.internal_buffer, sizeof(uint8_t) * 128);
		}
		DSK_STATE.op = DSK_OP_DONE;
		DSK_STATE.func = DSK_FUNC_IDLE;
		DSK_STATE.sector = 0;
		DSK_STATE.track = 0;
	}
}

void
clear_buffer_register()
{
	DSK_STATE.rxdb = 0;
}

void
set_status_register(uint16_t data)
{
	/* Only allow writeable fields to be set. */
	data &= RXCS_WRITE_MASK;
	DSK_STATE.rxcs |= data;
}

void
set_transfer_flag()
{
	if ((DSK_STATE.rxcs & RXCS_FLAG_XFER) == 0) {
		DSK_STATE.rxcs |= RXCS_FLAG_XFER;
	}
}

void
clear_transfer_flag()
{
	DSK_STATE.rxcs = DSK_STATE.rxcs & RXCS_FLAG_XFER;
}

uint8_t
is_transfer_flag_set()
{
	if (DSK_STATE.rxcs & RXCS_FLAG_XFER) {
		return 1;
	} else {
		return 0;
	}
}

void
set_done_flag()
{
	if ((DSK_STATE.rxcs & RXCS_FLAG_DONE) == 0) {
		DSK_STATE.rxcs |= RXCS_FLAG_DONE;
	}
}

/* XXX Unused?
void
clear_done_flag()
{
	if (DSK_STATE.rxcs & RXCS_FLAG_DONE) {
		DSK_STATE.rxcs ^= RXCS_FLAG_DONE;
	}
}
*/

/* XXX Unused?
uint8_t
is_done_flag_set()
{
	if (DSK_STATE.rxcs & RXCS_FLAG_DONE) {
		return 1;
	} else {
		return 0;
	}
}
*/

/* XXX Unused?
void
clear_writable_flags()
{
	DSK_STATE.rxcs = DSK_STATE.rxcs & ~RXCS_WRITE_MASK;
}
*/

void
clear_all_flags()
{
	DSK_STATE.rxcs = 0;
}

void
usage()
{
	fprintf(stderr, "disk -f <disk image>\n");
}

void
handler(int signo, siginfo_t *info, void *context)
{
	/* Signal and join bus threads. */
	if (BUS_STATE != NULL) {
		cleanup_bus(BUS_STATE);
	}

	shutdown_disk();

	dump_disk_state();

	exit(EXIT_SUCCESS);
}


int
main(int argc, char **argv)
{
	/* Setup signal handler. */
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

	/* Setup disk service. */
	int ret, opt;
	char *sock_l = "mem";
	char *sock_name = "disk";
	char *disk_image = NULL;

	while ((opt = getopt(argc, argv, "i:")) != -1) {
		switch (opt) {
		case 'i':
			disk_image = optarg;
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (disk_image == NULL) {
		usage();
		fprintf(stderr, "must specify a disk image\n");
		exit(EXIT_FAILURE);
	}

	pid_t pid = getpid();
	fprintf(stderr, "disk is starting [%d]\n", pid);

	BUS_STATE = init_bus(sock_l, sock_name, NULL);
	if (BUS_STATE == NULL) {
		fprintf(stderr, "failed to initialize bus state.\n");
		exit(EXIT_FAILURE);
	}
	BUS_STATE->addr_internal_to_device = &is_local_addr;

	ret = connect_device_bus(BUS_STATE);
	if (ret != 0) {
		fprintf(stderr, "failed to connect to bus.\n");
		exit(EXIT_FAILURE);
	}
	if (BUS_STATE == NULL) {
		fprintf(stderr, "failed to initialize bus state.\n");
		exit(EXIT_FAILURE);
	}

	init_disk();
	execute();
	shutdown_disk();

	return 0;
}

