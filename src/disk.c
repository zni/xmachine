#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/disk.h"

void exec_disk(disk_t*, memory_t*);

void fill_buffer(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (disk->index == SECTOR_SIZE) {
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_DONE);
        disk->index = 0;
        disk->state = S_DONE;
        disk->current_func = F_IDLE;
        return;
    }

    uint8_t data = 0;
    uint16_t xfer_status = (flags & RXCS_XFER);
    printf("\t%s(flags: %07o)\n", __FUNCTION__, flags);
    if (xfer_status != RXCS_XFER) {
        memory->direct_write_word_OR(memory, RXCS, flags);
        data = memory->direct_read_byte_n(memory, RXDB);
        printf("\tfetching data, %d: %04o\n", disk->index, data);
        disk->buffer[disk->index] = data;
        disk->index++;
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
    } else {
        printf("\twaiting for write to register\n");
        thrd_sleep(&(struct timespec){.tv_nsec=5000000}, NULL);
    }
}

void empty_buffer(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (disk->index == SECTOR_SIZE) {
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_DONE);
        disk->index = 0;
        disk->state = S_DONE;
        disk->current_func = F_IDLE;
        return;
    }

    uint16_t xfer_status = (flags & RXCS_XFER);
    if (xfer_status != RXCS_XFER) {
        memory->direct_write_word_OR(memory, RXCS, flags);
        memory->direct_write_byte_n(memory, RXDB, disk->index);
        disk->index++;
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
    }
}

void write_sector(disk_t *disk, memory_t *memory)
{
    if (disk->state == S_SECTOR) {

    }
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
    disk->state = S_BEGIN;

    disk->fill_buffer = &fill_buffer;
    disk->empty_buffer = &empty_buffer;
    disk->write_sector = &write_sector;
    disk->read_sector = &read_sector;

    return disk;
}

void free_disk(disk_t **disk)
{
    for (int i = 0; i < SECTOR_SIZE; i++) {
        printf("%02x ", (*disk)->buffer[i]);
        if (((i + 1) % 16) == 0 && i != 0) {
            putchar('\n');
        }
    }
    putchar('\n');

    free(*disk);
    *disk = NULL;
}

void clear_RXDB(memory_t *memory)
{
    memory->direct_write_word_n(memory, RXDB, 0);
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    memory->direct_write_word_n(memory, RXCS, flags ^ RXCS_XFER);
}

void clear_RXCS(memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    memory->direct_write_word_n(memory, RXCS, flags ^ RXCS_XFER);
}

void exec_disk(disk_t *disk, memory_t *memory)
{
    bool bus_shutdown = memory->bus_shutdown;
    uint16_t R_RXCS;
    uint16_t go;
    while (!bus_shutdown) {
        R_RXCS = memory->direct_read_word(memory, RXCS);
        go = RXCS_GO & R_RXCS;
        if (go && ((disk->state == S_BEGIN) || (disk->state == S_DONE))) {
            printf("go(%07o)\n", go);
            memory->direct_write_word(memory, RXCS, 0);

            uint16_t function = (RXCS_FS & R_RXCS) >> 1;

            switch (function) {
                case F_FILL_BUFFER:
                    disk->state = S_FILL;
                    disk->current_func = F_FILL_BUFFER;
                    memory->direct_write_word(memory, RXCS, RXCS_XFER);
                    break;
                case F_EMPTY_BUFFER:
                    disk->state = S_EMPTY;
                    disk->current_func = F_EMPTY_BUFFER;
                    break;
                case F_WRITE_SECTOR:
                    disk->state = S_SECTOR;
                    disk->current_func = F_WRITE_SECTOR;
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
            printf("%s(S_FILL, %07o)\n", __FUNCTION__, R_RXCS);
            disk->fill_buffer(disk, memory);

        } else if ((disk->state == S_EMPTY) &&
                   (disk->current_func == F_EMPTY_BUFFER)) {
            printf("%s(S_EMPTY, %07o)\n", __FUNCTION__, R_RXCS);
            thrd_sleep(&(struct timespec){.tv_nsec=5000000}, NULL);
            disk->empty_buffer(disk, memory);

        } else if (disk->state == S_DONE) {
            disk->state = S_BEGIN;
            printf("%s(flags: %07o)\n", __FUNCTION__, R_RXCS);
            memory->direct_write_word_OR(memory, RXCS, R_RXCS | RXCS_DONE);
            //thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
            thrd_sleep(&(struct timespec){.tv_nsec=5000000}, NULL);
        }
        bus_shutdown = memory->bus_shutdown;
    }
}

int start_disk_subsystem(void *disk_ss)
{
    disk_subsystem_t *ds = (disk_subsystem_t *) disk_ss;
    ds->memory->register_read_side_effect(ds->memory, RXDB, clear_RXDB);
    ds->memory->register_write_side_effect(ds->memory, RXDB, clear_RXCS);
    exec_disk(ds->disk, ds->memory);
    return 0;
}