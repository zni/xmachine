#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "memory.h"

#define RXCS 0177170
#define RXDB 0177172

#define RXCS_GO     0000001
#define RXCS_FS     0000016
#define RXCS_US     0000020
#define RXCS_DONE   0000040
#define RXCS_INT    0000100
#define RXCS_XFER   0000200

#define SECTOR_SIZE 128

typedef enum disk_func {
    F_FILL_BUFFER = 0,
    F_EMPTY_BUFFER,
    F_WRITE_SECTOR,
    F_READ_SECTOR,
    F_NOT_USED,
    F_READ_STATUS,
    F_WRITE_DEL,
    F_READ_ERR,
    F_IDLE
} disk_func_t;

struct disk;
typedef struct disk disk_t;
struct disk {
    uint8_t buffer[SECTOR_SIZE];
    uint8_t index;
    uint8_t track;
    uint8_t sector;

    disk_func_t current_func;

    void (*fill_buffer)(disk_t*);
    void (*empty_buffer)(disk_t*);
    void (*read_sector)(disk_t*);
    void (*write_sector)(disk_t*);
};

disk_t* new_disk();
void free_disk(disk_t**);

void exec_disk(memory_t*);

#endif