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

#define TKS_LOC 017500
#define TKB_LOC 017502
#define TKS_RDRENB 0000001
#define TKS_DONE   0000200
#define TKS_BUSY   0004000

#define TPS_LOC 017504
#define TPB_LOC 017506
#define TPS_READY  0000200


typedef struct machine_state {
/*
    uint8_t MEMORY[MEMSIZE];
    uint16_t PSW;

    uint16_t PC;
    uint16_t MAR;
    uint16_t MBR;

    uint16_t SP;
    uint8_t *SRCB;
    uint8_t *DESTB;
    uint16_t *SRC;
    uint16_t *DEST;
    uint16_t R[5];
*/
    uint16_t ALU;
    uint16_t IR;
    bool HALTED;
    memory_t *memory;
/*
    uint16_t *TKB;
    uint16_t *TKS;
    uint16_t *TPS;
    uint16_t *TPB;
*/
} machine_state_t;

#endif