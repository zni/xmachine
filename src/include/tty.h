#ifndef TTY_H
#define TTY_H

#include "machine_state.h"

#define TKS_LOC 017500
#define TKB_LOC 017502
#define TPS_LOC 017504
#define TPB_LOC 017506

#define TKS_RDRENB 0000001
#define TKS_DONE   0000200
#define TKS_BUSY   0004000
#define TPS_READY  0000200

void exec_tty_kb(machine_state_t*);
void exec_tty_print(machine_state_t*);

void init_tty_subsystem(machine_state_t*);
void shutdown_tty_subsystem();

#endif