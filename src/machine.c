#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMSIZE 4096
#define ZEROFLAG 0x0001

uint16_t MEMORY[MEMSIZE];
uint16_t PSR = 0;
uint16_t IR = 0;
uint16_t PC = 0;
uint16_t MAR = 0;
uint16_t MBR = 0;
uint16_t ALU = 0;
uint16_t R[5] = { 0, 0, 0, 0, 0 };

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
        PSR |= ZEROFLAG;
    } else {
        PSR = 0;
    }
}

void run_machine(void)
{
    bool halted = false;

    while (!halted) {
        MBR = PC;
        MAR = MEMORY[MBR];
        IR = (MAR & 0170000) >> 12;
        if (IR == 0) {
            IR = (MAR & 0077000) >> 9;
            if (IR == 0) {
                IR = (MAR & 0007700) >> 6;
            }
        }

        switch (IR) {
            case 00: // HALT
                halted = true;
                break;
            case 050: // CLR
                R[MAR & 077] = 0;
                PC++;
                break;
            case 053: // DEC
                R[MAR & 077]--;
                PC++;
                break;
            case 052: // INC
                R[MAR & 077]++;
                PC++;
                break;
            case 054: // NEG
                MBR = MAR & 077;
                R[MBR] = ~(R[MBR]) + 1;
                PC++;
                break;
            case 051: // COM
                MBR = MAR & 077;
                R[MBR] = ~R[MBR];
                PC++;
                break;
            case 062: // ASR
                MBR = MAR & 077;
                R[MBR] >>= 1;
                PC++;
                break;
            case 063: // ASL
                MBR = MAR & 077;
                R[MBR] <<= 1;
                PC++;
                break;
            case 01: // MOV
                if ((MAR & 07700) == 02700) { // Immediate addressing.
                    PC++;
                    MBR = PC;
                    R[MAR & 077] = MEMORY[MBR];
                    PC++;
                } else { // Register to register.
                    R[MAR & 077] = R[(MAR & 07700) >> 6];
                }
                PC++;
                break;
            case 02: // CMP
                ALU = R[MAR & 077];
                ALU -= R[(MAR & 07700) >> 6];
                PC++;
                set_zero_flag();
                break;
            case 06: // ADD
                ALU = R[MAR & 077];
                ALU += R[(MAR & 07700) >> 6];
                R[MAR & 077] = ALU;
                PC++;
                break;
            case 016: // SUB
                ALU = R[MAR & 077];
                ALU -= R[(MAR & 07700) >> 6];
                R[MAR & 077] = ALU;
                PC++;
                set_zero_flag();
                break;
        }


    }
}

void dump_state(uint16_t program_len)
{
    printf("PSR: 0o%06o\n", PSR);
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