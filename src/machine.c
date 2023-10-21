#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEMSIZE 4096
#define PROGSIZE 7
#define ZEROFLAG 0x0001

uint16_t MEMORY[MEMSIZE];
uint16_t PWR = 0;
uint16_t IR = 0;
uint16_t PC = 0;
uint16_t MAR = 0;
uint16_t MBR = 0;
uint16_t ALU = 0;
uint16_t R[5] = { 0, 0, 0, 0, 0 };

uint16_t PROGRAM[PROGSIZE] = {
    0x1005,
    0x2004,
    0x3005,
    0x6006,
    0x4,
    0x1,
    0x0000
};

uint16_t PROGRAM2[PROGSIZE] = {
    0010100,
    0060101,
    0020101,
    0160000,
    0000000,
    0000000,
    0000000
};

void zero_memory(void)
{
    for (int i = 0; i < MEMSIZE; i++) { MEMORY[i] = 0; }
}

void load_program(void)
{
    for (int i = 0; i < PROGSIZE; i++) { MEMORY[i] = PROGRAM2[i]; }
}

void set_zero_flag(void)
{
    if (ALU == 0) {
        PWR |= ZEROFLAG;
    } else {
        PWR = 0;
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
            IR = (MAR & 0007000) >> 9;
            if (IR == 0) {
                IR = (MAR & 0000700) >> 6;
            }
        }

        switch (IR) {
            case 00: // HALT
                halted = true;
                break;
            case 050: // CLR
                MBR = MAR & 077;
                R[MBR] = 0;
                PC++;
                break;
            case 053: // DEC
                MBR = MAR & 077;
                R[MBR]--;
                PC++;
                break;
            case 052: // INC
                MBR = MAR & 077;
                R[MBR]++;
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
                R[MAR & 077] = R[(MAR & 07700) >> 6];
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

void dump_state(void)
{
    printf("PWR: 0o%06o\n", PWR);
    printf("IR : 0o%06o\n", IR);
    printf("PC : 0o%06o\n", PC);
    printf("MAR: 0o%06o\n", MAR);
    printf("MBR: 0o%06o\n", MBR);
    printf("ALU: 0o%06o\n", ALU);
    for (int i = 0; i < 5; i++) {
        printf("R%d : 0o%06o\n", i, R[i]);
    }

    for (int i = 0; i < PROGSIZE; i++) {
        printf("0o%06o ", MEMORY[i]);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    // Put some initial values in the registers until we sort out loading them.
    R[0] = 2;
    R[1] = 5;

    zero_memory();
    load_program();
    run_machine();
    dump_state();

    return 0;
}