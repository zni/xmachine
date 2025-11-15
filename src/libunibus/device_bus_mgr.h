#ifndef LIBBUSD_DEVICE_BUS_MGR_H
#define LIBBUSD_DEVICE_BUS_MGR_H

#include "../common/include/types.h"
#include "bus_state.h"
#include "priority_bus.h"
#include "data_bus.h"

#define SOCKET_DIR "/tmp/xmachine"

// Bus init and cleanup.
bus_state_t* init_bus(char*, char*, char*);
void cleanup_bus(bus_state_t*);
int connect_bus(bus_state_t*);

// Priority functions.
void req_bus_master(bus_state_t*);
void release_bus_master(bus_state_t*);

// Master functions.
uint16_t read_data_in(bus_state_t*, uint32_t);
void write_data_out(bus_state_t*, uint32_t, uint16_t);

// Slave functions.
void data_bus_check(bus_state_t*, data_op_t*);
void data_bus_cont(bus_state_t*, data_op_t*);

#endif

