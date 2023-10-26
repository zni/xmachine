#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/handlers.h"
#include "include/machine_state.h"

machine_state_t *STATE;

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
    machine->SRCB = NULL;
    machine->DESTB = NULL;

    for (int i = 0; i < REGISTERS; i++) { machine->R[i] = 0;}
    for (int i = 0; i < MEMSIZE; i++) { machine->MEMORY[i] = 0; }
}

void load_program(machine_state_t *machine, char *program)
{
    FILE *binary = fopen(program, "r");
    if (binary == NULL) {
        fprintf(stderr, "Failed to open binary file.\n");
        exit(1);
    }

    uint16_t len = 0;
    fread(&len, sizeof(uint16_t), 1, binary);

    uint16_t load_offset = 0;
    fread(&load_offset, sizeof(uint16_t), 1, binary);

    uint16_t program_length = load_offset + (len * 2);
    for (int i = load_offset; i < program_length; i++) {
        fread(&(machine->MEMORY[i]), sizeof(uint8_t), 1, binary);
    }
    fclose(binary);

    machine->PC = load_offset;
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

            printf("0o%05o: ", r);
            for (int i = 0; i < 16; i++) {
                printf("0o%03o ", row[i]);
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
        // Words are stored little endian.
        machine->MAR = machine->MEMORY[machine->MBR + 1] << 8;
        machine->MAR |= machine->MEMORY[machine->MBR];
        machine->IR = machine->MAR;

        exec_instruction(machine);
    }
}

/*
 * If we get here, we're spiraling out of control.
 * So, halt the machine, even though at this point it basically is halted.
 * Dump the state of the machine and exit.
 */
void handle_user_interrupt(int signo)
{
    fprintf(stderr, "HALTING\n");
    HALT(STATE);
    dump_state(STATE, true);

    exit(1);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: machine <binary>\n");
        return 1;
    }

    machine_state_t machine;
    STATE = &machine;

    // Setup a signal handler for SIGINT.
    struct sigaction sa;
    sa.sa_flags = SIGINFO;
    sa.sa_handler = &handle_user_interrupt;
    sigaction(SIGINT, &sa, NULL);


    init_machine(&machine);
    load_program(&machine, argv[1]);
    run_machine(&machine);
    dump_state(&machine, true);

    return 0;
}