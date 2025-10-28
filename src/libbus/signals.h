#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdint.h>

#define ASSERTED 1
#define NEGATED 0

#define SOCK_NAME_LEN 100

typedef uint8_t assert_t;

typedef enum signal {
    // Data Bus Signal Types
    A,
    D,
    C0,
    C1,
    MSYN,
    SSYN,
    PA,
    PB,
    INTR,

    // Priority Bus Signal Types
    BR,
    BG,
    NPR,
    NPG,
    SACK,
    BBSY,

    // Initialization Bus Signal Types
    INIT,
    ACLO,
    DCLO
} signal_t;

typedef struct bus_req {
    /*
     * True Unibus wouldn't know this, but since I'm making my own Rube
     * Goldberg version of it, I need to know.
     */
    char from[SOCK_NAME_LEN];

    signal_t sig;
    assert_t assertion;
    uint32_t value;
} bus_req_t;

#endif

