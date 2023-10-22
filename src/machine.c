#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/machine.h"

uint16_t MEMORY[MEMSIZE];
uint16_t PSW = 0;
uint16_t IR  = 0;
uint16_t PC  = 0;
uint16_t MAR = 0;
uint16_t MBR = 0;
uint16_t ALU = 0;
uint16_t SP  = 0;
uint16_t *DEST = NULL;
uint16_t *SRC  = NULL;
uint16_t R[5] = { 0, 0, 0, 0, 0 };
bool HALTED = false;

void exec_halt(void)
{
    HALTED = true;
}

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

void setup_dest_addressing(void)
{
    // From immediate.
    if ((IR & 077) == 027) {
        PC++;
        MBR = PC;
        DEST = &(MEMORY[MBR]);

    // From absolute.
    } else if ((IR & 077) == 037) {
        PC++;
        MBR = PC;
        MAR = MEMORY[MBR];
        DEST = &(MEMORY[MAR]);

    // From register.
    } else if ((IR & 070) == 00) {
        DEST = &(R[(IR & 007)]);
    }
}

void setup_src_addressing(void)
{
    // From immediate address.
    if ((IR & 07700) == 02700) {
        PC++;
        MBR = PC;
        SRC = &(MEMORY[MBR]);

    // From absolute address.
    } else if ((IR & 03700) == 03700) {
        PC++;
        MBR = PC;
        MAR = MEMORY[MBR];
        SRC = &(MEMORY[MAR]);

    // From register.
    } else if ((IR & 07000) == 0) {
        SRC = &(R[(IR & 00700) >> 6]);
    }
}

void exec_special_double_operand(void)
{
    SRC = &(R[(IR & 0700) >> 6]);
    setup_dest_addressing();

    switch ((IR & 0177000) >> 9) {
        case MUL:
            break;

        case DIV:
            break;

        case ASH:
            break;

        case ASHC:
            break;

        case XOR:
            ALU = *SRC;
            ALU ^= *DEST;
            *DEST = ALU;
            PC++;
            break;

        default:
            PC++;
            break;
    }
}

void exec_double_operand(void)
{
    setup_src_addressing();
    setup_dest_addressing();

    switch ((IR & 0170000) >> 12) {
        case MOV:
            *DEST = *SRC;
            PC++;
            break;

        case MOVB:
            *DEST = (*SRC & 0377);
            PC++;
            break;

        case CMP:
            ALU = *DEST;
            ALU -= *SRC;
            PC++;
            set_zero_flag();
            break;

        case CMPB:
            break;

        case BIT:
            ALU = *DEST;
            ALU &= *SRC;
            *DEST = ALU;
            set_zero_flag();
            PC++;
            break;

        case BITB:
            break;

        case BIC:
            ALU = *DEST;
            ALU &= ~(*SRC);
            *DEST = ALU;
            PC++;
            break;

        case BICB:
            break;

        case BIS:
            ALU = *DEST;
            ALU |= *SRC;
            *DEST = ALU;
            PC++;
            break;

        case BISB:
            break;

        case ADD:
            ALU = *DEST;
            ALU += *SRC;
            *DEST = ALU;
            PC++;
            break;

        case SUB:
            ALU = *DEST;
            ALU -= *SRC;
            *DEST = ALU;
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
    setup_dest_addressing();

    switch ((IR & 0177700) >> 6) {
        case JMP:
            PC = *DEST;
            break;

        case SWAB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 0177400) >> 8) | ((*DEST & 000377) << 8);
            PC++;
            break;

        case CLR:
            *DEST = 0;
            PC++;
            break;

        case CLRB:
            MBR = MAR & 077;
            *DEST = (*DEST & 0177400);
            PC++;
            break;

        case COM:
            MBR = MAR & 077;
            *DEST = ~(*DEST);
            PC++;
            break;

        case COMB:
            MBR = MAR & 077;
            *DEST = ~(*DEST & 000377) | (*DEST & 0177400);
            PC++;
            break;

        case INC:
            (*DEST)++;
            PC++;
            break;

        case INCB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) + 1) | (*DEST & 0177400);
            PC++;
            break;

        case DEC:
            (*DEST)--;
            PC++;
            break;

        case DECB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) - 1) | (*DEST & 0177400);
            PC++;
            break;

        case NEG:
            MBR = MAR & 077;
            *DEST = ~(*DEST) + 1;
            PC++;
            break;

        case NEGB:
            MBR = MAR & 077;
            *DEST = ((~(*DEST & 000377) + 1) & 000377) | (*DEST & 0177400);
            PC++;
            break;

        case ADC:
            MBR = MAR & 077;
            *DEST = *DEST + (PSW & CARRYFLAG);
            PC++;
            break;

        case ADCB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) + (PSW & CARRYFLAG)) | (*DEST & 0177400);
            PC++;
            break;

        case SBC:
            MBR = MAR & 077;
            *DEST = *DEST - (PSW & CARRYFLAG);
            PC++;
            break;

        case SBCB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) - (PSW & CARRYFLAG)) | (*DEST & 0177400);
            PC++;
            break;

        case TST:
            break;

        case TSTB:
            break;

        case ROR:
            MBR = MAR & 077;
            *DEST = (*DEST << 1) | (*DEST >> 15);
            PC++;
            break;

        case RORB:
            MBR = MAR & 077;
            *DEST = (((*DEST & 000377) << 1) | ((*DEST & 000377) >> 7)) | (*DEST & 0177400);
            PC++;
            break;

        case ROL:
            MBR = MAR & 077;
            *DEST = (*DEST << 15) | (*DEST >> 1);
            PC++;
            break;

        case ROLB:
            MBR = MAR & 077;
            *DEST = (((*DEST & 000377) << 7) | ((*DEST & 000377) >> 1)) | (*DEST & 0177400);
            break;

        case ASR:
            MBR = MAR & 077;
            ALU = *DEST;
            ALU >>= 1;
            *DEST = ALU;
            PC++;
            break;

        case ASRB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) >> 1) | (*DEST & 0177400);
            PC++;
            break;

        case ASL:
            MBR = MAR & 077;
            ALU = *DEST;
            ALU <<= 1;
            *DEST = ALU;
            PC++;
            break;

        case ASLB:
            MBR = MAR & 077;
            *DEST = ((*DEST & 000377) << 1) | (*DEST & 0177400);
            PC++;
            break;

        case MTPS:
            MBR = MAR & 077;
            PSW = *DEST;
            PC++;
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
            if (PSW & NEGATIVEFLAG) {
                *DEST = -1;
            } else {
                *DEST = 0;
            }
            PC++;
            break;

        case MFPS:
            MBR = MAR & 077;
            *DEST = PSW;
            PC++;
            break;

        default: // Catch undefined opcodes.
            PC++;
            break;
    }
}

void dump_state(bool dump_memory)
{
    printf("PSW: 0o%06o\n", PSW);
    printf("IR : 0o%06o\n", IR);
    printf("PC : 0o%06o\n", PC);
    printf("MAR: 0o%06o\n", MAR);
    printf("MBR: 0o%06o\n", MBR);
    printf("ALU: 0o%06o\n", ALU);
    printf("SP : 0o%06o\n", SP);
    for (int i = 0; i < 5; i++) {
        printf("R%d : 0o%06o\n", i, R[i]);
    }

    if (dump_memory) {
        uint16_t row[16];
        bool all_zero = true;
        for (int r = 0; r < MEMSIZE; r += 16) {
            for (int c = 0; c < 16; c++) {
                if (MEMORY[r+c] != 0) { all_zero = false; }
                row[c] = MEMORY[r+c];
            }
            if (all_zero && ((r + 16) < MEMSIZE)) {
                continue;
            }

            printf("0o%06o: ", r);
            for (int i = 0; i < 16; i++) {
                printf("0o%06o ", row[i]);
            }
            printf("\n");
            all_zero = true;
        }
    }
    printf("\n");
}

void run_machine(void)
{
    while (!HALTED) {
        MBR = PC;
        MAR = MEMORY[MBR];
        IR = MAR;

        // Is this a double operand op?
        if (((IR & 0170000) >> 12) != 0) {
            exec_double_operand();
        } else {
            // Is this a special double operand op?
            if (((IR & 0177000) >> 9) != 0 &&
                (((IR & 0177000) >> 9) >= 070 && ((IR & 0177000) >> 9) <= 074)) {
                exec_special_double_operand();
            } else {
                // Is this a single operand op?
                if (((IR & 0177700) >> 6) == 0) {
                    exec_halt();
                } else {
                    exec_single_operand();
                }
            }
        }
    }
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
    dump_state(true);

    free(program);

    return 0;
}