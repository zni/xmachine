#include <stdio.h>
#include <stdlib.h>

#include "debug.h"


void
dbg_bus(bus_state *bus, const char *msg)
{
#ifdef DBG_LIBUNIBUS
	fprintf(stderr, "%s: %s\n", bus->sock, msg);
#endif
}

