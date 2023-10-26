#ifndef MACHINE_STATE_H
#define MACHINE_STATE_H

#include <stdbool.h>
#include <stdint.h>

#define MEMSIZE 8192
#define REGISTERS 5

#define IPLBITS         0340
#define TRAPFLAG        0020
#define NEGATIVEFLAG    0010
#define ZEROFLAG        0004
#define OVERFLOWFLAG    0002
#define CARRYFLAG       0001

typedef struct machine_state {
    uint8_t MEMORY[MEMSIZE];
    uint16_t PSW;
    uint16_t IR;
    uint16_t PC;
    uint16_t MAR;
    uint16_t MBR;
    uint16_t ALU;
    uint16_t SP;
    uint8_t *SRCB;
    uint8_t *DESTB;
    uint16_t *SRC;
    uint16_t *DEST;
    uint16_t R[5];
    bool HALTED;
} machine_state_t;

#endif