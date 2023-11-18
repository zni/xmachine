#ifndef MACHINE_STATE_H
#define MACHINE_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

#include "memory.h"
#include "disk.h"

// Processor Status Fields
#define CURRMODE        0140000
#define PREVMODE        0030000
#define IPLBITS         0000340
#define TRAPFLAG        0000020
#define NEGATIVEFLAG    0000010
#define ZEROFLAG        0000004
#define OVERFLOWFLAG    0000002
#define CARRYFLAG       0000001


typedef struct machine_state {
    uint16_t ALU;
    uint16_t IR;
    bool HALTED;
    bool STEP;
    memory_t *memory;
    disk_subsystem_t disk_ss;
    thrd_t disk_thread;
} machine_state_t;

#endif