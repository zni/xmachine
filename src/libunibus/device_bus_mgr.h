#ifndef LIBUNIBUS_DEVICE_BUS_MGR_H
#define LIBUNIBUS_DEVICE_BUS_MGR_H

#include "bus_state.h"

#define SOCKET_DIR "/tmp/xmachine"

/* Bus init and cleanup. */
bus_state* init_bus(char*, char*, char*);
void cleanup_bus(bus_state*);
int connect_cpu_bus(bus_state*);
int connect_device_bus(bus_state*);

/* Priority functions. */
void req_bus_master(bus_state*);
void release_bus_master(bus_state*);

/* Master functions. */
uint16_t read_data_in(bus_state*, uint32_t);
void write_data_out(bus_state*, uint32_t, uint16_t);
void write_data_out_b(bus_state*, uint32_t, uint8_t);

/* Slave functions. */
void data_bus_check(bus_state*, data_xfer_spec*);
void data_bus_reply(bus_state*, data_xfer_spec*);

#endif

