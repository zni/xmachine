#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "../common/include/types.h"

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
write_word(mem_t *mem, uint32_t addr, uint16_t word)
{
    mem->mar = addr;
    mem->mbr = word;
    mem->store[mem->mar] = mem->mbr & 0377;
    mem->store[mem->mar + 1] = (mem->mbr & 0177400) >> 8;
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

int
main(int argc, char **argv)
{
    mem_t *mem = init_mem();
    if (mem == NULL) {
        fprintf(stderr, "Failed to initialize memory.\n");
        exit(EXIT_FAILURE);
    }

    // TODO Become a daemon.

    dump_mem(mem);

    free(mem);

    return 0;
}

