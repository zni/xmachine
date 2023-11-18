#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/disk.h"

void exec_disk(disk_t*, memory_t*);

/**
 * Fill the disk buffer with bytes from the RXDB register.
 * This precedes a call to `write_sector`.
 */
void fill_buffer(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (disk->index == SECTOR_SIZE) {
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
        if (disk->index != SECTOR_SIZE)
            memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
        else
            memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_DONE);
    } else {
        printf("\twaiting for write to register\n");
        thrd_sleep(&(struct timespec){.tv_nsec=5000000}, NULL);
    }
}

/**
 * Empty the disk's buffer following a read from the disk to RXDB.
 */
void empty_buffer(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (disk->index == SECTOR_SIZE) {
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
        if (disk->index != SECTOR_SIZE)
            memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
        else
            memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_DONE);
    }
}

/**
 * Write the data currently held in the disk buffer to disk.
 */
void write_sector(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    uint16_t xfer_status = (flags & RXCS_XFER);
    if (disk->state == S_SECTOR && (xfer_status != RXCS_XFER)) {
        disk->sector = memory->direct_read_byte_n(memory, RXDB);
        disk->state = S_TRACK;
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
        printf("\t%s(SECTOR, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );
    } else if (disk->state == S_TRACK && (xfer_status != RXCS_XFER)) {
        disk->track = memory->direct_read_byte_n(memory, RXDB);
        disk->state = S_WRITE;
        printf("\t%s(TRACK, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );
    } else if (disk->state == S_WRITE) {
        printf("\t%s(WRITE, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );

        if (disk->disk_drive != NULL) {
            // 3328 bytes per track.
            uint32_t track_offset = 3328 * disk->track;
            uint32_t disk_offset = track_offset + (128 * disk->sector);
            fseek(disk->disk_drive, disk_offset, SEEK_SET);
            fwrite(disk->buffer, sizeof(uint8_t), 128, disk->disk_drive);
        }

        disk->state = S_DONE;
        disk->current_func = F_IDLE;
        disk->sector = 0;
        disk->track = 0;
    }
}

/**
 * Read data from the disk into the disk buffer.
 */
void read_sector(disk_t *disk, memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    uint16_t xfer_status = (flags & RXCS_XFER);
    if (disk->state == S_SECTOR && (xfer_status != RXCS_XFER)) {
        disk->sector = memory->direct_read_byte_n(memory, RXDB);
        disk->state = S_TRACK;
        memory->direct_write_word_OR(memory, RXCS, flags ^ RXCS_XFER);
        printf("\t%s(SECTOR, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );
    } else if (disk->state == S_TRACK && (xfer_status != RXCS_XFER)) {
        disk->track = memory->direct_read_byte_n(memory, RXDB);
        disk->state = S_READ;
        printf("\t%s(TRACK, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );
    } else if (disk->state == S_READ) {
        printf("\t%s(READ, sector: %04o, track: %04o)\n",
            __FUNCTION__,
            disk->sector,
            disk->track
        );

        if (disk->disk_drive != NULL) {
            // 3328 bytes per track.
            uint32_t track_offset = 3328 * disk->track;
            uint32_t disk_offset = track_offset + (128 * disk->sector);
            fseek(disk->disk_drive, disk_offset, SEEK_SET);
            fread(disk->buffer, sizeof(uint8_t), 128, disk->disk_drive);
        }

        disk->state = S_DONE;
        disk->current_func = F_IDLE;
        disk->sector = 0;
        disk->track = 0;
    }
}

/**
 * Initialize a new `disk_t` struct.
 */
disk_t* new_disk(char *disk_image)
{
    disk_t *disk = malloc(sizeof(disk_t));
    memset(disk->buffer, 0, sizeof(uint8_t) * SECTOR_SIZE);
    disk->sector = 0;
    disk->track = 0;
    disk->index = 0;

    if (disk_image == NULL) {
        disk->disk_drive = NULL;
    } else {
        disk->disk_drive = fopen(disk_image, "r+");
    }

    disk->current_func = F_IDLE;
    disk->state = S_BEGIN;

    disk->fill_buffer = &fill_buffer;
    disk->empty_buffer = &empty_buffer;
    disk->write_sector = &write_sector;
    disk->read_sector = &read_sector;

    return disk;
}

/**
 * Free the memory in use by the `disk_t` struct.
 */
void free_disk(disk_t **disk)
{
    for (int i = 0; i < SECTOR_SIZE; i++) {
        printf("%02x ", (*disk)->buffer[i]);
        if (((i + 1) % 16) == 0 && i != 0) {
            putchar('\n');
        }
    }
    putchar('\n');

    if ((*disk)->disk_drive != NULL) {
        fclose((*disk)->disk_drive);
    }

    free(*disk);
    *disk = NULL;
}

/**
 * Side effect read handler for RXDB.
 * Zeroes RXDB and removes the TR flag after a read.
 */
void clear_RXDB(memory_t *memory)
{
    memory->direct_write_word_n(memory, RXDB, 0);
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (flags & RXCS_XFER) {
        flags ^= RXCS_XFER;
    }
    memory->direct_write_word_n(memory, RXCS, flags);
}

/**
 * Side effect write handler for RXDB.
 * Clears the TR flag on write to RXDB.
 */
void clear_RXCS(memory_t *memory)
{
    uint16_t flags = memory->direct_read_word(memory, RXCS);
    if (flags & RXCS_XFER) {
        flags ^= RXCS_XFER;
    }
    memory->direct_write_word_n(memory, RXCS, flags);
}

/**
 * The main driver for the disk subsystem.
 * Process incoming flag commands and call the
 * related functions.
 */
void exec_disk(disk_t *disk, memory_t *memory)
{
    bool bus_shutdown = memory->bus_shutdown;
    uint16_t R_RXCS;
    uint16_t go;
    while (!bus_shutdown) {
        R_RXCS = memory->direct_read_word(memory, RXCS);
        go = RXCS_GO & R_RXCS;
        if (go && ((disk->state == S_BEGIN) || (disk->state == S_DONE))) {
            printf("go(%07o)\n", R_RXCS);
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
                    memory->direct_write_word(memory, RXCS, RXCS_XFER);
                    break;
                case F_READ_SECTOR:
                    disk->state = S_SECTOR;
                    disk->current_func = F_READ_SECTOR;
                    memory->direct_write_word(memory, RXCS, RXCS_XFER);
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
            disk->empty_buffer(disk, memory);

        } else if ((disk->current_func == F_READ_SECTOR)) {
            printf("%s(F_READ_SECTOR, %07o)\n", __FUNCTION__, R_RXCS);
            disk->read_sector(disk, memory);

        } else if ((disk->current_func == F_WRITE_SECTOR)) {
            printf("%s(F_WRITE_SECTOR, %07o)\n", __FUNCTION__, R_RXCS);
            disk->write_sector(disk, memory);

        } else if (disk->state == S_DONE) {
            disk->state = S_BEGIN;
            printf("%s(S_DONE, flags: %07o)\n", __FUNCTION__, R_RXCS);
            memory->direct_write_word_OR(memory, RXCS, R_RXCS | RXCS_DONE);
            thrd_sleep(&(struct timespec){.tv_nsec=5000000}, NULL);
        }
        bus_shutdown = memory->bus_shutdown;
    }
}

/**
 * The function to be passed to `thrd_create`.
 */
int start_disk_subsystem(void *disk_ss)
{
    disk_subsystem_t *ds = (disk_subsystem_t *) disk_ss;
    ds->memory->register_read_side_effect(ds->memory, RXDB, clear_RXDB);
    ds->memory->register_write_side_effect(ds->memory, RXDB, clear_RXCS);
    exec_disk(ds->disk, ds->memory);
    return 0;
}