#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"
#include "../common/include/types.h"
#include "../libload/load.h"
#include "../libunibus/device_bus_mgr.h"

mem_t *MEM_STATE = NULL;
bus_state_t *BUS_STATE = NULL;

uint16_t read_word(mem_t*, uint32_t);
void write_word(mem_t*, uint32_t, uint16_t);
void write_byte(mem_t*, uint32_t, uint16_t);
void dump_mem(mem_t*);

void
usage()
{
    fprintf(stderr, "mem -f <a.out file> -o <load offset>\n");
}

bool_t
is_local_addr(uint32_t addr)
{
    if (addr >= MEMLOW && addr <= MEMHIGH) {
        return TRUE;
    }

    return FALSE;
}

void
handler(int signo, siginfo_t *info, void *context)
{
    if (MEM_STATE != NULL) {
        dump_mem(MEM_STATE);
        free(MEM_STATE);
    }

    /* Signal and join bus threads. */
    cleanup_bus(BUS_STATE);

    exit(EXIT_SUCCESS);
}

void
load_aout(uint8_t *buffer, exec_t *header, uint32_t offset)
{
    int n, m;
    for (n = 0, m = offset; n < header->a_text; n++, m++) {
        MEM_STATE->store[m] = buffer[n];
    }
}

mem_t*
init_mem()
{
    mem_t *mem = malloc(sizeof(mem_t));
    if (mem == NULL) {
        perror("mem malloc");
        return NULL;
    }

    return mem;
}

void
perform_read(data_op_t *d_op)
{
    d_op->value = read_word(MEM_STATE, d_op->addr);
}

void
perform_write(data_op_t *d_op)
{
    write_word(MEM_STATE, d_op->addr, d_op->value);
}

void
perform_writeb(data_op_t *d_op)
{
}

void
write_word(mem_t *mem, uint32_t addr, uint16_t word)
{
    mem->mar = addr;
    mem->mbr = word;
    mem->store[mem->mar] = mem->mbr & 0377;
    mem->store[mem->mar + 1] = (mem->mbr & 0177400) >> 8;
}

void
write_byte(mem_t *mem, uint32_t addr, uint16_t word)
{
    mem->mar = addr;
    mem->mbr = word;
    mem->store[mem->mar] = mem->mbr & 0377;
}

uint16_t
read_word(mem_t *mem, uint32_t addr)
{
    uint16_t word = 0;
    word = mem->store[addr];
    word |= mem->store[addr + 1] << 8;
    return word;
}

void
dump_mem(mem_t *mem)
{
    uint16_t row[16];
    uint8_t all_zero = TRUE;
    for (int r = 0; r < MEMWORDS; r += 32) {
        for (int c = 0, i = 0; c < 32; c += 2, i++) {
            if (read_word(mem, r + c) != 0) { all_zero = FALSE; }
            row[i] = read_word(mem, r + c);
        }
        if (all_zero && ((r + 16) < MEMWORDS) && (r != 0)) {
            continue;
        }

        printf("0o%05o: ", r);
        for (int i = 0; i < 16; i++) {
            printf("0o%07o ", row[i]);
        }
        putchar('\n');
        all_zero = TRUE;
    }
}

void
execute()
{
    data_op_t slave_req;
    while(TRUE) {
        printf("mem: execute: blocking...\n");
        pthread_mutex_lock(&(BUS_STATE->perma_slave_mutex));
        pthread_cond_wait(&(BUS_STATE->cond_perma_slave), &(BUS_STATE->perma_slave_mutex));
        pthread_mutex_unlock(&(BUS_STATE->perma_slave_mutex));
        printf("mem: execute: unblocked\n");

        // Check for bus requests.
        printf("mem: execute: checking requests\n");
        data_bus_check(BUS_STATE, &slave_req);
        switch (slave_req.op) {
        case R_NONE:
            printf("mem: execute: got R_NONE\n");
            continue;
        case R_BLOCK_IN:
            printf("mem: execute: got R_BLOCK_IN\n");
            perform_read(&slave_req);
            break;
        case R_BLOCK_OUT:
            printf("mem: execute: got R_BLOCK_OUT\n");
            perform_write(&slave_req);
            break;
        case R_BLOCK_OUTB:
            printf("mem: execute: got R_BLOCK_OUTB\n");
            perform_writeb(&slave_req);
            break;
        default:
            printf("mem: execute: base case, err\n");
            continue;
        }
        printf("mem: execute: calling data_bus_cont\n");
        data_bus_cont(BUS_STATE, &slave_req);
        printf("mem: execute: done calling data_bus_cont\n");
    }
}

int
main(int argc, char **argv)
{
    int ret, opt;
    uint32_t load_offset = 0;
    char sock_l[] = "cpu";
    char sock_name[] = "mem";
    char *aout_file = NULL;
    exec_t *aout_header = NULL;
    uint8_t *aout_buffer = NULL;

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

    pid_t pid = getpid();
    fprintf(stderr, "mem is starting [%d]\n", pid);

    mem_t *mem = init_mem();
    if (mem == NULL) {
        fprintf(stderr, "Failed to initialize memory.\n");
        exit(EXIT_FAILURE);
    }
    MEM_STATE = mem;

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
        free(mem);
        exit(EXIT_FAILURE);
    }

    aout_header = aout_header_read(aout_file);
    if (aout_header == NULL) {
        printf("failed to read a.out header\n");
        free(mem);
        exit(EXIT_FAILURE);
    }

    aout_buffer = aout_text_read(aout_file, aout_header);
    if (aout_buffer == NULL) {
        printf("failed to load TEXT from a.out\n");
        free(mem);
        exit(EXIT_FAILURE);
    }
    load_aout(aout_buffer, aout_header, load_offset);

    bus_state_t *bus = init_bus(sock_l, sock_name, NULL);
    if (bus == NULL) {
        fprintf(stderr, "Failed to initialize bus.\n");
        exit(EXIT_FAILURE);
    }
    bus->addr_internal_to_device = &is_local_addr;

    BUS_STATE = bus;
    ret = connect_bus(BUS_STATE);
    if (ret != 0) {
        fprintf(stderr, "Failed to connect to bus.\n");
        exit(EXIT_FAILURE);
    }
    execute();

    return 0;
}

