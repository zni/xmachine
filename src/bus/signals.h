#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdint.h>

#define ASSERTED 1
#define NEGATED 0

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
    signal_t sig;
    uint8_t assertion;
    uint32_t value;
} bus_req_t;

#endif

