#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/handlers.h"
#include "include/machine_state.h"

machine_state_t *STATE;



void exec_halt(machine_state_t *machine)
{
    machine->HALTED = true;
}

void init_machine(machine_state_t *machine)
{
    machine->ALU = 0;
    machine->MAR = 0;
    machine->MBR = 0;
    machine->PC = 0;
    machine->PSW = 0;
    machine->SP = 0;
    machine->IR = 0;
    machine->HALTED = false;
    machine->SRC = NULL;
    machine->DEST = NULL;

    for (int i = 0; i < REGISTERS; i++) { machine->R[i] = 0;}
    for (int i = 0; i < MEMSIZE; i++) { machine->MEMORY[i] = 0; }
}

void load_program(machine_state_t *machine, uint16_t *program, uint16_t len)
{
    for (int i = 0; i < len; i++) { machine->MEMORY[i] = program[i]; }
}

void dump_state(machine_state_t *machine, bool dump_memory)
{
    printf("PSW: 0o%06o\n", machine->PSW);
    printf("IR : 0o%06o\n", machine->IR);
    printf("PC : 0o%06o\n", machine->PC);
    printf("MAR: 0o%06o\n", machine->MAR);
    printf("MBR: 0o%06o\n", machine->MBR);
    printf("ALU: 0o%06o\n", machine->ALU);
    printf("SP : 0o%06o\n", machine->SP);
    for (int i = 0; i < 5; i++) {
        printf("R%d : 0o%06o\n", i, machine->R[i]);
    }

    if (dump_memory) {
        uint16_t row[16];
        bool all_zero = true;
        for (int r = 0; r < MEMSIZE; r += 16) {
            for (int c = 0; c < 16; c++) {
                if (machine->MEMORY[r+c] != 0) { all_zero = false; }
                row[c] = machine->MEMORY[r+c];
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

void run_machine(machine_state_t *machine)
{
    uint16_t high_bit;
    uint16_t index_tier_1;
    uint16_t index_tier_2;
    uint16_t index_tier_3;
    uint16_t branch_bit;
    while (!machine->HALTED) {
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->IR = machine->MAR;

        /*
         * I've spent so long on this, I'm not sure what I was trying to gain anymore.
         * 
         * Basically, grab the high bit, if it's set we're in the high op codes.
         * From there, look at each position of the op code. They become our indexes
         * into the jump tables. Once they're established, do some slight of hand,
         * and voila we are calling our op code of choice.
         * 
         * Benefits of this:
         * Can swap out handlers as needed, if needed, probably YAGNI.
         * 
         * Downsides:
         * Brain slowly turned to mush.
         */

        high_bit     = (machine->IR & 0100000) >> 15;
        index_tier_1 = (machine->IR & 0070000) >> 12;
        index_tier_2 = (machine->IR & 0007000) >> 9;
        index_tier_3 = (machine->IR & 0000700) >> 6;
        branch_bit   = (machine->IR & 0000400) >> 8;

        if (high_bit != 0) {
            if (index_tier_1 != 0) {
                // The high index is only set for a specific group of opcodes.
                index_tier_2 = index_tier_1;
                index_tier_1 = 7;

                high_opcodes[index_tier_1][index_tier_2](machine);
            } else {
                index_tier_1 = index_tier_2;
                index_tier_2 = index_tier_3;

                // If we are between index 0 and 3, we have a branch instruction.
                // So look at the branch bit for our index.
                if ((index_tier_1 >= 0) && (index_tier_1 <= 3)) {
                    index_tier_2 = branch_bit ? 4 : 0;
                }

                high_opcodes[index_tier_1][index_tier_2](machine);
            }
        } else {
            if (index_tier_1 != 0) {
                index_tier_2 = index_tier_1 - 1;
                index_tier_1 = 8;
                low_opcodes[index_tier_1][index_tier_2](machine);
            } else if (index_tier_2 != 0) {
                index_tier_1 = index_tier_2;
                index_tier_2 = index_tier_3;

                // If we are between index 1 and 3, we have a branch instruction.
                // Look at the branch bit to determine our second index.
                if ((index_tier_1) >= 1 && (index_tier_1 <= 3)) {
                    index_tier_2 = branch_bit ? 4 : 0;
                }
                low_opcodes[index_tier_1][index_tier_2](machine);
            } else if (index_tier_3 != 0) {
                index_tier_1 = 0;
                index_tier_2 = index_tier_3;
                low_opcodes[index_tier_1][index_tier_2](machine);
            } else {
                exec_halt(machine);
            }
        }
    }
}

/*
 * If we get here, there was most likely an unknown op code encountered.
 * So, halt the machine, even though at this point it basically is halted.
 * Dump the state of the machine and exit.
 */
void handle_unknown_opcode(int signo)
{
    fprintf(stderr, "HALTING\n");
    exec_halt(STATE);
    dump_state(STATE, true);

    exit(1);
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

    machine_state_t machine;
    STATE = &machine;

    // Setup a signal handler for SIGSEGV and SIGBUS,
    // as unknown opcodes seem to be able to trigger both.
    struct sigaction sa;
    sa.sa_flags = SIGINFO;
    sa.sa_sigaction = NULL;
    sa.sa_handler = &handle_unknown_opcode;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    init_machine(&machine);
    load_program(&machine, program, len);
    run_machine(&machine);
    dump_state(&machine, true);

    free(program);

    return 0;
}