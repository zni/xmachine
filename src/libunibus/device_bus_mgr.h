#ifndef LIBBUSD_DEVICE_BUS_MGR_H
#define LIBBUSD_DEVICE_BUS_MGR_H

#include "../common/include/types.h"
#include "bus_state.h"
#include "priority_bus.h"

bus_state_t* init_bus(char*, char*, char*);
void req_bus_master(bus_state_t*);
void release_bus_master(bus_state_t*);
int init_pr_bus(bus_state_t*);

#endif

