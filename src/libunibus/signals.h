#ifndef SIGNALS_H
#define SIGNALS_H

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

typedef enum data_bus_msg {
    DBM_REQ,
    DBM_RESP
} data_bus_msg_t;

/* I'm lazy, so the control lines (C0, C1) are just mnemonics now. */
typedef enum d_signal {
    D_DATI,
    D_DATIP,
    D_DATO,
    D_DATOB,
    D_EMPTY
} d_signal_t;

typedef struct data_bus_req {
    char from[SOCK_NAME_LEN];
    data_bus_msg_t msg_type;

    d_signal_t c;
    uint32_t addr;
    uint16_t data;

    assert_t msyn;
    assert_t ssyn;
} data_bus_req_t;

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

