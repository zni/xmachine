#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define SECTOR_SIZE 128
#define BUFFER_SIZE 128
#define RXCS_WRITE_MASK   040137

enum DiskControllerAddresses {
    RXCS = 0777170,
    RXDB = 0777172
};

//enum class RXCSFlag {
//    GO   = 0000001,
//    FS   = 0000016,
//    US   = 0000020,
//    DONE = 0000040,
//    INT  = 0000100,
//    XFER = 0000200
//};

//enum class DiskControllerState {
//    BEGIN,
//    SECTOR,
//    TRACK,
//    FILL,
//    EMPTY,
//    WRITE_SECTOR,
//    READ_SECTOR,
//    DONE
//};

//enum class DiskControllerFunction {
//    FILL_BUFFER = 0,
//    EMPTY_BUFFER,
//    WRITE_SECTOR,
//    READ_SECTOR,
//    NOT_USED,
//    READ_STATUS,
//    WRITE_DEL,
//    READ_ERR,
//    IDLE
//};


#endif

