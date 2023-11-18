#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "include/machine_state.h"
#include "include/memory.h"
#include "include/tty.h"

struct termios TERMINAL_DEFAULT, XMACHINE_TERMINAL;

/* Teletype Keyboard Functions */

void clear_TKB(memory_t *m)
{
    m->direct_write_word(m, TKB_LOC, 0);
}

void clear_flags_TKS(memory_t *m)
{
    m->direct_write_word(m, TKS_LOC, 0);
}

void read_TKB(memory_t *m)
{
    clear_TKB(m);
    clear_flags_TKS(m);
}

void fill_buffer_TKB(machine_state_t *machine)
{
    uint16_t TKB = (uint16_t) fgetc(stdin);
    machine->memory->direct_write_word(machine->memory, TKB_LOC, TKB);
}

void exec_tty_kb(machine_state_t *machine)
{
    machine->memory->src = TKS_LOC;
    machine->memory->dest = TKS_LOC;
    uint16_t TKS = machine->memory->read_word(machine->memory);
    if (TKS & TKS_RDRENB) {
        TKS ^= TKS_BUSY;
        machine->memory->write_word(machine->memory, TKS);

        fill_buffer_TKB(machine);

        TKS ^= TKS_DONE;
        TKS ^= TKS_BUSY;
        TKS ^= TKS_RDRENB;
        machine->memory->write_word(machine->memory, TKS);
    }
}

/* Teleprinter Functions */

uint8_t buffer_filled_TPB(machine_state_t *machine)
{
    uint8_t buffer = machine->memory->direct_read_byte(machine->memory, TPB_LOC);
    return (buffer != 0);
}

void clear_flags_TPS(machine_state_t *machine)
{
    machine->memory->direct_write_word(machine->memory, TPS_LOC, 0);
}

void clear_TPB(memory_t *m)
{
    uint16_t TPS = m->direct_read_word(m, TPS_LOC);
    m->direct_write_word(m, TPB_LOC, 0);
    m->direct_write_word(m, TPS_LOC, TPS | TPS_READY);
}

uint8_t fetch_buffer_TPB(machine_state_t *machine)
{
    return machine->memory->direct_read_byte(machine->memory, TPB_LOC);
}

void exec_tty_print(machine_state_t *machine)
{
    uint16_t TPS = machine->memory->direct_read_word(machine->memory, TPS_LOC);
    if ((TPS & TPS_READY)) {
        clear_flags_TPS(machine);
        uint8_t c = fetch_buffer_TPB(machine);
        if (c != 0)
            fputc(c, stdout);
    }
}

void init_tty_subsystem(machine_state_t *machine)
{
    // Teletype Keyboard Effects
    machine->memory->register_read_side_effect(machine->memory, TKB_LOC, read_TKB);

    // Teleprinter Effects
    machine->memory->register_read_side_effect(machine->memory, TPB_LOC, clear_TPB);

    // Set the READY for the first time.
    machine->memory->direct_write_word(machine->memory, TPS_LOC, TPS_READY);

    // Let's mess up the terminal. Canonical mode and echo be-gone!
    tcgetattr(STDIN_FILENO, &TERMINAL_DEFAULT);
    XMACHINE_TERMINAL = TERMINAL_DEFAULT;
    XMACHINE_TERMINAL.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &XMACHINE_TERMINAL);
}

void shutdown_tty_subsystem()
{
    // Back to normal.
    tcsetattr(STDIN_FILENO, TCSANOW, &TERMINAL_DEFAULT);
}