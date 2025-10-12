#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#define MEMBYTES 65535
#define MEMWORDS 32767

#define MEMLOW 0
#define MEMHIGH MEMBYTES

typedef struct mem {
    uint16_t mar;
    uint16_t mbr;
    uint8_t store[MEMBYTES];
} mem_t;

#endif
