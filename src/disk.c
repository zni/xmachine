#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/disk.h"

void exec_disk(disk_t*, memory_t*);

void fill_buffer(disk_t *disk, memory_t *memory)
{
    printf("%s()\n", __FUNCTION__);
}

void empty_buffer(disk_t *disk, memory_t *memory)
{
    printf("%s()\n", __FUNCTION__);
}

void write_sector(disk_t *disk, memory_t *memory)
{
    printf("%s()\n", __FUNCTION__);
}

void read_sector(disk_t *disk, memory_t *memory)
{
    printf("%s()\n", __FUNCTION__);
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

void exec_disk(disk_t *disk, memory_t *memory)
{
    bool bus_shutdown = memory->bus_shutdown;

    while (!bus_shutdown) {
        uint16_t R_RXCS = memory->direct_read_word(memory, RXCS);
        uint16_t go = RXCS_GO & R_RXCS;
        if (go && disk->state == S_BEGIN) {
            uint16_t function = (RXCS_FS & R_RXCS) >> 1;

            switch (function) {
                case F_FILL_BUFFER:
                    disk->state = S_FILL;
                    disk->current_func = F_FILL_BUFFER;
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
        } else if ((disk->state == S_FILL) &&
                   (disk->current_func == F_FILL_BUFFER)) {
            disk->fill_buffer(disk, memory);
        }

        bus_shutdown = memory->bus_shutdown;
    }
}

int start_disk_subsystem(void *disk_ss)
{
    disk_subsystem_t *ds = (disk_subsystem_t *) disk_ss;
    exec_disk(ds->disk, ds->memory);
    return 0;
}