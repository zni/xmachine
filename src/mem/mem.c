#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../libload/load.h"
#include "../libunibus/device_bus_mgr.h"

#ifdef MEMDBG
#define MEM_P(...) fprintf(stderr, __VA_ARGS__)
#else
#define MEM_P(...)
#endif


#define MEMBYTES 65535
#define MEMWORDS 32767

#define MEMLOW 0
#define MEMHIGH MEMBYTES

typedef struct _mem {
	uint16_t mar;
	uint16_t mbr;
	uint8_t store[MEMBYTES];
} mem_dev;

mem_dev *MEM_STATE = NULL;
bus_state *BUS_STATE = NULL;

static void usage();
static uint8_t is_local_addr(uint32_t);
static void handler(int, siginfo_t*, void*);
static void load_aout(aout_object*, uint32_t);
static void init_mem();
static void perform_read(data_xfer_spec*);
static void perform_write(data_xfer_spec*);
static void perform_writeb(data_xfer_spec*);
static void write_word(uint32_t, uint16_t);
static void write_byte(uint32_t, uint16_t);
static uint16_t read_word(uint32_t);
static void dump_mem();
static void execute();

void
usage()
{
	fprintf(stderr, "mem -f <a.out file> -o <load offset>\n");
}

uint8_t
is_local_addr(uint32_t addr)
{
	if (addr >= MEMLOW && addr <= MEMHIGH) {
		return 1;
	}

	return 0;
}

void
handler(int signo, siginfo_t *info, void *context)
{
	if (MEM_STATE != NULL) {
		dump_mem();
		free(MEM_STATE);
	}

	/* Signal and join bus threads. */
	cleanup_bus(BUS_STATE);

	exit(EXIT_SUCCESS);
}

void
load_aout(aout_object *aout, uint32_t offset)
{
	MEM_P("loading program\n");

	int n, m;
	for (n = 0, m = offset; n < (aout->header.a_text >> 1); n++) {
		MEM_P("[0o%06o] = 0o%06o\n", m, aout->text[n]);

		MEM_STATE->store[m] = (aout->text[n] & 0x00FF);
		m++;
		MEM_STATE->store[m] = (aout->text[n] & 0xFF00) >> 8;
		m++;
	}
}

void
init_mem()
{
	MEM_STATE = (mem_dev*) malloc(sizeof(mem_dev));
	if (MEM_STATE == NULL) {
		perror("mem malloc");
	}
}

void
perform_read(data_xfer_spec *d_op)
{
	d_op->value = read_word(d_op->addr);
}

void
perform_write(data_xfer_spec *d_op)
{
	write_word(d_op->addr, d_op->value);
}

void
perform_writeb(data_xfer_spec *d_op)
{
	write_byte(d_op->addr, d_op->value);
}

void
write_word(uint32_t addr, uint16_t word)
{
	MEM_P("W [0o%06o] = 0o%06o\n", addr, word);

	MEM_STATE->mar = addr;
	MEM_STATE->mbr = word;
	MEM_STATE->store[MEM_STATE->mar] = MEM_STATE->mbr & 0377;
	MEM_STATE->store[MEM_STATE->mar + 1] = (MEM_STATE->mbr & 0177400) >> 8;
}

void
write_byte(uint32_t addr, uint16_t word)
{
	MEM_STATE->mar = addr;
	MEM_STATE->mbr = word;
	MEM_STATE->store[MEM_STATE->mar] = MEM_STATE->mbr & 0377;
}

uint16_t
read_word(uint32_t addr)
{
	uint16_t word = 0;
	word = MEM_STATE->store[addr];
	word |= MEM_STATE->store[addr + 1] << 8;
	return word;
}

void
dump_mem()
{
	uint16_t row[16];
	uint8_t all_zero = 1;
	for (int r = 0; r < MEMWORDS; r += 32) {
		for (int c = 0, i = 0; c < 32; c += 2, i++) {
			if (read_word(r + c) != 0) { all_zero = 0; }
			row[i] = read_word(r + c);
		}
		if (all_zero && ((r + 16) < MEMWORDS) && (r != 0)) {
			continue;
		}

		printf("0o%05o: ", r);
		for (int i = 0; i < 16; i++) {
			printf("0o%07o ", row[i]);
		}
		putchar('\n');
		all_zero = 1;
	}
}

void
execute()
{
	data_xfer_spec slave_req;
	while(1) {
		pthread_mutex_lock(&(BUS_STATE->perma_slave_mutex));
		pthread_cond_wait(&(BUS_STATE->cond_perma_slave), &(BUS_STATE->perma_slave_mutex));
		pthread_mutex_unlock(&(BUS_STATE->perma_slave_mutex));

		// Check for bus requests.
		data_bus_check(BUS_STATE, &slave_req);
		switch (slave_req.op) {
		case R_NONE:
			continue;
		case R_BLOCK_IN:
			perform_read(&slave_req);
			break;
		case R_BLOCK_OUT:
			perform_write(&slave_req);
			break;
		case R_BLOCK_OUTB:
			perform_writeb(&slave_req);
			break;
		default:
			continue;
		}
		data_bus_reply(BUS_STATE, &slave_req);
	}
}

int
main(int argc, char **argv)
{
	int ret, opt;
	uint32_t load_offset = 0;
	char *sock_l = "cpu";
	char *sock_name = "mem";
	char *aout_file = NULL;
	aout_object *aout = NULL;

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

	init_mem();
	if (MEM_STATE == NULL) {
		fprintf(stderr, "Failed to initialize memory.\n");
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "f:o:")) != -1) {
		switch (opt) {
		case 'f':
			aout_file = optarg;
			break;
		case 'o':
			load_offset = strtoul(optarg, NULL, 10);
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (aout_file == NULL) {
		usage();
		printf("must specify an a.out file\n");
		free(MEM_STATE);
		exit(EXIT_FAILURE);
	}

	aout = aout_read(aout_file);
	if (aout == NULL) {
		printf("failed to read a.out header\n");
		free(MEM_STATE);
		exit(EXIT_FAILURE);
	}

	load_aout(aout, load_offset);

	pid_t pid = getpid();
	fprintf(stderr, "mem is starting [%d]\n", pid);

	BUS_STATE = init_bus(sock_l, sock_name, NULL);
	if (BUS_STATE == NULL) {
		fprintf(stderr, "Failed to initialize bus.\n");
		exit(EXIT_FAILURE);
	}
	BUS_STATE->addr_internal_to_device = &is_local_addr;

	ret = connect_device_bus(BUS_STATE);
	if (ret != 0) {
		fprintf(stderr, "Failed to connect to bus.\n");
		exit(EXIT_FAILURE);
	}
	execute();

	return 0;
}

