#ifndef MACHINE_STATE_H
#define MACHINE_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "memory.h"

#define IPLBITS         0340
#define TRAPFLAG        0020
#define NEGATIVEFLAG    0010
#define ZEROFLAG        0004
#define OVERFLOWFLAG    0002
#define CARRYFLAG       0001


typedef struct machine_state {
    uint16_t ALU;
    uint16_t IR;
    bool HALTED;
    bool STEP;
    memory_t *memory;
} machine_state_t;

#endif