#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include "include/handlers.h"
#include "include/machine_state.h"
#include "include/objreader.h"
#include "include/tty.h"

machine_state_t *STATE;

void init_machine(machine_state_t *machine, bool step, char *disk_image)
{
    machine->ALU = 0;
    machine->IR = 0;
    machine->HALTED = false;
    machine->STEP = step;

    machine->memory = NULL;
    initialize_memory(&(machine->memory));
    machine->memory->bus_shutdown = false;

    init_tty_subsystem(machine);
    machine->disk_ss.disk = new_disk(disk_image);
    machine->disk_ss.memory = machine->memory;
    thrd_create(&machine->disk_thread, start_disk_subsystem, &(machine->disk_ss));
}

void load_program_from_mcode(machine_state_t *machine, char *program)
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

    uint8_t byte = 0;
    uint16_t program_length = load_offset + (len * 2);
    for (int i = load_offset; i < program_length; i++) {
        fread(&(byte), sizeof(uint8_t), 1, binary);
        machine->memory->direct_write_byte(machine->memory, i, byte);
    }
    fclose(binary);

    machine->memory->set_r(machine->memory, R_PC, load_offset);
}

void dump_state(machine_state_t *machine, bool dump_memory)
{

    printf("PSW: 0o%07o\n", machine->memory->get_r(machine->memory, R_PS));
    printf("PC : 0o%07o\n", machine->memory->get_r(machine->memory, R_PC));
    printf("SP : 0o%07o\n", machine->memory->get_r(machine->memory, R_SP));
    printf("IR : 0o%07o\n", machine->IR);
    printf("ALU: 0o%07o\n", machine->ALU);

    for (int i = 0; i < 6; i++) {
        printf("R%d : 0o%07o\n", i, machine->memory->get_r(machine->memory, i));
    }

    if (dump_memory) {
        uint16_t row[16];
        bool all_zero = true;
        for (int r = 0; r < MEMWORDS; r += 32) {
            for (int c = 0, i = 0; c < 32; c += 2, i++) {
                if (machine->memory->direct_read_word(machine->memory, r+c) != 0) { all_zero = false; }
                row[i] = machine->memory->direct_read_word(machine->memory, r+c);
            }
            if (all_zero && ((r + 16) < MEMWORDS) && (r != 0)) {
                continue;
            }

            printf("0o%05o: ", r);
            for (int i = 0; i < 16; i++) {
                printf("0o%07o ", row[i]);
            }
            printf("\n");
            all_zero = true;
        }
    }
    printf("\n");
}

void run_machine(machine_state_t *machine)
{
    while (!machine->HALTED) {
        uint16_t PC = machine->memory->get_r(machine->memory, R_PC);
        machine->memory->src = PC;
        machine->IR = machine->memory->read_word(machine->memory);
        machine->memory->word_advance_r(machine->memory, R_PC);

        if (machine->STEP) {
            dump_state(machine, true);
            getc(stdin);
        }

        exec_instruction(machine);
        exec_tty_kb(machine);
        exec_tty_print(machine);
    }

    machine->memory->bus_shutdown = machine->HALTED;
}

void shutdown_machine(machine_state_t *machine)
{
    thrd_join(machine->disk_thread, NULL);
    destroy_memory_mtx();

    free_memory(&machine->memory);
    machine->memory = NULL;


    free_disk(&(machine->disk_ss.disk));
    shutdown_tty_subsystem();
}

/*
 * If we get here, we're spiraling out of control.
 * Halt the machine.
 */
void handle_user_interrupt(int signo)
{
    fprintf(stderr, "HALTING\n");
    HALT(STATE);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: machine -l -s <binary>\n");
        return 1;
    }

    char *disk = NULL;
    bool step = false;
    bool list_only = false;
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[1]) < 2 && argv[i][0] != '-') {
            continue;
        }

        if (strncmp("-l", argv[i], 2) == 0) {
            list_only = true;
            break;
        } else if (strncmp("-s", argv[i], 2) == 0) {
            step = true;
        } else if (strncmp("-d", argv[i], 2) == 0) {
            disk = argv[i];
            disk++;
            disk++;
        }
    }

    machine_state_t machine;
    STATE = &machine;

#if (defined(_XOPEN_SOURCE) || defined(__APPLE__))
    // Setup a signal handler for SIGINT.
    struct sigaction sa;
    sa.sa_flags = SIGINFO;
    sa.sa_handler = &handle_user_interrupt;
    sigaction(SIGINT, &sa, NULL);
#elif (defined(__WIN32__) || (defined(__unix__)))
    // Use the old style signal handler.
    signal(SIGINT, &handle_user_interrupt);
#endif

    if (list_only) {
        init_machine(&machine, step, disk);
        load_program_from_obj(&machine, argv[argc - 1]);
        dump_state(&machine, true);
        shutdown_machine(&machine);
    } else {
        init_machine(&machine, step, disk);
        // load_program_from_mcode(&machine, argv[1]);
        load_program_from_obj(&machine, argv[argc - 1]);
        run_machine(&machine);
        dump_state(&machine, true);
        shutdown_machine(&machine);
    }

    return 0;
}