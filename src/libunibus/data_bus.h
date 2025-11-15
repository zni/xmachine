#ifndef LIBBUSD_DATA_BUS_H
#define LIBBUSD_DATA_BUS_H

#include <stdint.h>
#include "bus_state.h"

void* data_bus_mgr(void*);
void in_word(data_state_t*);
void out_word(data_state_t*);
void out_byte(data_state_t*);

#endif

