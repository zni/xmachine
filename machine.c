#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEMSIZE 4096
#define PROGSIZE 7

uint16_t MEMORY[MEMSIZE];
uint16_t IR = 0;
uint16_t PC = 0;
uint16_t MAR = 0;
uint16_t MBR = 0;
uint16_t ALU = 0;
uint16_t R[5] { 0, 0, 0, 0, 0 };

/*
    0000 0000 0000 0000
*/

// 0x0 HALT
// 0x1 LOAD
// 0x2 ADD
// 0x3 STORE
// 0x4 BEQ
// 0x5 BNE
// 0x6 JMP

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
    0
}

void load_program(void)
{
    for (int i = 0; i < PROGSIZE; i++) { MEMORY[i] = PROGRAM[i]; }
}

void run_machine(void)
{
    bool halted = false;

    while (!halted) {
        MBR = PC;
        MAR = MEMORY[MBR];
        IR = (MAR & 0177700) >> 6;

        switch (IR) {
            case 0x0: // HALT
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
                R[MBR] = ~R[MBR] + 1;
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
            case 01: // MOV p75
                R[MAR & 077] = R[(MAR & 07700) >> 6];
                PC++;
                break;
/*
            case 0x1: // LOAD
                MBR = MAR & 0x0FFF;
                MAR = MEMORY[MBR];
                AC = MAR;
                PC++;
                break;
            case 0x2: // ADD
                MBR = MAR & 0x0FFF;
                MAR = MEMORY[MBR];
                ALU = AC;
                ALU += MAR;
                AC = ALU;
                PC++;
                break;
            case 0x3: // STORE
                MBR = MAR & 0x0FFF;
                MEMORY[MBR] = AC;
                PC++;
                break;
            case 0x4: // BEQ
                if (AC == 0) {
                    MBR = MAR & 0x0FFF;
                    PC = MBR;
                } else {
                    PC++;
                }
                break;
            case 0x5: // BNE
                if (AC != 0) {
                    MBR = MAR & 0x0FFF;
                    PC = MBR;
                } else {
                    PC++;
                }
                break;
            case 0x6: // JMP
                MBR = MAR & 0x0FFF;
                PC = MBR;
                break;
*/
        }


    }
}

void dump_state(void)
{
    printf("IR : 0x%04x\n", IR);
    printf("PC : 0x%04x\n", PC);
    printf("AC : 0x%04x\n", AC);
    printf("MAR: 0x%04x\n", MAR);
    printf("MBR: 0x%04x\n", MBR);
    printf("ALU: 0x%04x\n", ALU);

    for (int i = 0; i < PROGSIZE; i++) {
        printf("0x%04x ", MEMORY[i]);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    for (int i = 0; i < MEMSIZE; i++) { MEMORY[i] = 0; }

    load_program();

    run_machine();

    dump_state();

    return 0;
}