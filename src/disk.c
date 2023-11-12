#include <stdlib.h>
#include <string.h>
#include "include/disk.h"

void fill_buffer(disk_t *disk)
{

}

void empty_buffer(disk_t *disk)
{

}

void write_sector(disk_t *disk)
{

}

void read_sector(disk_t *disk)
{

}

disk_t* new_disk()
{
    disk_t *disk = malloc(sizeof(disk_t));
    memset(disk->buffer, 0, sizeof(uint8_t) * SECTOR_SIZE);
    disk->sector = 0;
    disk->track = 0;
    disk->index = 0;

    disk->current_func = F_IDLE;

    disk->fill_buffer = &fill_buffer;
    disk->empty_buffer = &empty_buffer;
    disk->write_sector = &write_sector;
    disk->read_sector = &read_sector;

    return disk;
}

void free_disk(disk_t **disk)
{
    free(*disk);
    *disk = NULL;
}

void exec_disk(memory_t *memory)
{
    uint16_t R_RXCS = memory->direct_read_word(memory, RXCS);
    uint16_t go = RXCS_GO & R_RXCS;
    if (go) {
        uint16_t function = (RXCS_FS & R_RXCS) >> 1;

        switch (function) {
            case F_FILL_BUFFER:
                break;
            case F_EMPTY_BUFFER:
                break;
            case F_WRITE_SECTOR:
                break;
            case F_READ_SECTOR:
                break;
            case F_NOT_USED:
                break;
            case F_READ_STATUS:
                break;
            case F_WRITE_DEL:
                break;
            case F_READ_ERR:
                break;
            default:
                break;
        }
    }
}