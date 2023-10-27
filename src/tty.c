#include <ctype.h>
#include <stdio.h>
#include "include/machine_state.h"

/* Teletype Keyboard Functions */

void clear_TKB(machine_state_t *machine)
{
    *machine->TKB = 0;
}

void clear_flags_TKS(machine_state_t *machine)
{
    *machine->TKS = 0;
}

uint16_t read_TKB(machine_state_t *machine)
{
    uint16_t b = *machine->TKB;
    clear_TKB(machine);
    clear_flags_TKS(machine);
    return b;
}

void fill_buffer_TKB(machine_state_t *machine)
{
    *machine->TKB = (uint16_t) fgetc(stdin);
}

void exec_tty_kb(machine_state_t *machine)
{
    if (*machine->TKS & TKS_RDRENB) {
        *machine->TKS ^= TKS_BUSY;
        fill_buffer_TKB(machine);
        *machine->TKS ^= TKS_DONE;
        *machine->TKS ^= TKS_BUSY;
        *machine->TKS ^= TKS_RDRENB;
    }
}

/* Teleprinter Functions */

uint8_t buffer_filled_TPB(machine_state_t *machine)
{
    return *machine->TPB != 0;
}

void clear_flags_TPS(machine_state_t *machine)
{
    *machine->TPS = 0;
}

uint8_t fetch_and_clear_buffer_TPB(machine_state_t *machine)
{
    uint8_t c = (uint8_t) *machine->TPB;
    *machine->TPB = 0;
    return c;
}

void exec_tty_print(machine_state_t *machine)
{
    if ((*machine->TPS & TPS_READY) && buffer_filled_TPB(machine)) {
        clear_flags_TPS(machine);
        uint8_t c = fetch_and_clear_buffer_TPB(machine);
        fputc(c, stdout);
        *machine->TPS ^= TPS_READY;
    }
}