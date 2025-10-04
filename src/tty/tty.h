#ifndef TTY_H
#define TTY_H

#include <stdint.h>

#define TKS_WRITE_MASK 0000101
#define TKS_MODE_MASK  0004201
#define TPS_WRITE_MASK 0000100

// Based on registers from ASR 33 TELETYPE.
// Documentation found in PDP11 Peripherals and Interfacing Handbook
// Beginning Chapter 2, pg. 5

enum TTYRegisters {
    TKS = 0777560,
    TKB = 0777562,
    TPS = 0777564,
    TPB = 0777566
};

//enum class TKSFlag {
//    RDRENB    = 0000001,
//    INTERRUPT = 0000100,
//    DONE      = 0000200,
//    BUSY      = 0004000
//};
//
//enum class TPSFlag {
//    INTERRUPT = 0000100,
//    READY     = 0000200
//};

#endif
