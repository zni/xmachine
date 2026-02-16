#ifndef LIBUNIBUS_SIGNALS_H
#define LIBUNIBUS_SIGNALS_H

#include <stdint.h>

#define ASSERTED 1
#define NEGATED 0

#define SOCK_NAME_LEN 100

typedef uint8_t assert_t;

/*
 * Data Bus Signal Types
 * A,
 * D,
 * C0,
 * C1,
 * MSYN,
 * SSYN,
 * PA,
 * PB,
 * INTR,
 */


/*
 * Initialization Bus Signal Types
 * INIT,
 * ACLO,
 * DCLO
 */

typedef enum pr_signal {
	BR,
	BG,
	NPR,
	NPG,
	SACK,
	BBSY
} pr_signal_t;

typedef struct pr_bus_req {
	/*
	 * True Unibus wouldn't know this, but since I'm making my own Rube
	 * Goldberg version of it, I need to know.
	 */
	char from[SOCK_NAME_LEN];

	pr_signal_t sig;
	assert_t assertion;
	uint32_t value;
} pr_bus_req_t;

#endif

