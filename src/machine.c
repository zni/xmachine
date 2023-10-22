#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/machine.h"

#define MEMSIZE 4096
#define ZEROFLAG 0004

uint16_t MEMORY[MEMSIZE];
uint16_t PSW = 0;
uint16_t IR = 0;
uint16_t PC = 0;
uint16_t MAR = 0;
uint16_t MBR = 0;
uint16_t ALU = 0;
uint16_t R[5] = { 0, 0, 0, 0, 0 };
bool HALTED = false;

void zero_memory(void)
{
    for (int i = 0; i < MEMSIZE; i++) { MEMORY[i] = 0; }
}

void load_program(uint16_t *program, uint16_t len)
{
    for (int i = 0; i < len; i++) { MEMORY[i] = program[i]; }
}

void set_zero_flag(void)
{
    if (ALU == 0) {
        PSW |= ZEROFLAG;
    } else {
        PSW = (PSW & ZEROFLAG) ? (PSW ^ ZEROFLAG) : PSW;
    }
}

void exec_double_operand(void)
{
    switch (IR) {
        case MOV:
            if ((MAR & 07700) == 02700) { // Immediate addressing.
                PC++;
                MBR = PC;
                R[MAR & 077] = MEMORY[MBR];
                PC++;
            } else { // Register to register.
                R[MAR & 077] = R[(MAR & 07700) >> 6];
                PC++;
            }
            break;

        case MOVB:
            break;

        case CMP:
            ALU = R[MAR & 077];
            ALU -= R[(MAR & 07700) >> 6];
            PC++;
            set_zero_flag();
            break;

        case CMPB:
            break;

        case BIT:
            break;
        case BITB:
            break;
        case BIC:
            break;
        case BICB:
            break;
        case BIS:
            break;
        case BISB:
            break;

        case ADD:
            ALU = R[MAR & 077];
            ALU += R[(MAR & 07700) >> 6];
            R[MAR & 077] = ALU;
            PC++;
            break;

        case SUB:
            ALU = R[MAR & 077];
            ALU -= R[(MAR & 07700) >> 6];
            R[MAR & 077] = ALU;
            PC++;
            set_zero_flag();
            break;

        default: // Catch undefined opcodes.
            PC++;
            break;
    }
}

void exec_single_operand(void)
{
    switch (IR) {
        case JMP:
            // Register
            if ((MAR & 070) == 000) {
                PC = R[MAR & 07];

            // Immediate
            } else if ((MAR & 077) == 027) {
                PC++;
                MBR = PC;
                PC = MEMORY[MBR];
            }
        case SWAB:
            break;

        case CLR:
            R[MAR & 077] = 0;
            PC++;
            break;

        case CLRB:
            break;

        case COM:
            break;

        case COMB:
            break;

        case INC:
            R[MAR & 077]++;
            PC++;
            break;

        case INCB:
            break;

        case DEC:
            R[MAR & 077]--;
            PC++;
            break;

        case DECB:
            break;

        case NEG:
            MBR = MAR & 077;
            R[MBR] = ~(R[MBR]) + 1;
            PC++;
            break;

        case NEGB:
            break;

        case ADC:
            break;

        case ADCB:
            break;

        case SBC:
            break;

        case SBCB:
            break;

        case TST:
            break;

        case TSTB:
            break;

        case ROR:
            break;

        case RORB:
            break;

        case ROL:
            break;

        case ROLB:
            break;

        case ASR:
            MBR = MAR & 077;
            R[MBR] >>= 1;
            PC++;
            break;

        case ASRB:
            break;

        case ASL:
            MBR = MAR & 077;
            R[MBR] <<= 1;
            PC++;
            break;

        case ASLB:
            break;

        case MTPS:
            break;

        case MFPI:
            break;

        case MFPD:
            break;

        case MTPI:
            break;

        case MTPD:
            break;

        case SXT:
            break;

        case MFPS:
            break;

        default: // Catch undefined opcodes.
            PC++;
            break;
    }
}

void exec_halt(void)
{
    HALTED = true;
}

void run_machine(void)
{


    while (!HALTED) {
        MBR = PC;
        MAR = MEMORY[MBR];
        IR = (MAR & 0170000) >> 12;
        if (IR != 0) {
            exec_double_operand();
        } else {
            IR = (MAR & 0177000) >> 9;
            if (IR != 0 && (IR >= 070 && IR <= 074)) {
                printf("NOT IMPLEMENTED: %03o\n", IR);
            } else {
                IR = (MAR & 0177700) >> 6;
                if (IR == 0) {
                    exec_halt();
                } else {
                    exec_single_operand();
                }
            }
        }
    }
}

void dump_state(uint16_t program_len)
{
    printf("PSW: 0o%06o\n", PSW);
    printf("IR : 0o%06o\n", IR);
    printf("PC : 0o%06o\n", PC);
    printf("MAR: 0o%06o\n", MAR);
    printf("MBR: 0o%06o\n", MBR);
    printf("ALU: 0o%06o\n", ALU);
    for (int i = 0; i < 5; i++) {
        printf("R%d : 0o%06o\n", i, R[i]);
    }

    for (int i = 0; i < program_len; i++) {
        printf("0o%06o ", MEMORY[i]);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: machine <binary>\n");
        return 1;
    }

    FILE *binary = fopen(argv[1], "r");
    if (binary == NULL) {
        fprintf(stderr, "Failed to open binary file.\n");
        return 1;
    }

    uint16_t len = 0;
    fread(&len, sizeof(uint16_t), 1, binary);

    uint16_t *program = malloc(sizeof(uint16_t) * len);
    if (program == NULL) {
        fprintf(stderr, "Failed to allocate program.\n");
        return 1;
    }

    for (int i = 0; i < len; i++) {
        fread(&(program[i]), sizeof(uint16_t), 1, binary);
    }
    fclose(binary);

    zero_memory();
    load_program(program, len);
    run_machine();
    dump_state(len);

    free(program);

    return 0;
}