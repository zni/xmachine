#ifndef DISKCONTROLLER_H
#define DISKCONTROLLER_H

#include <cstdint>
#include <cstdio>

#include "Bus.h"
#include "IBusElement.h"

#define SECTOR_SIZE 128
#define BUFFER_SIZE 128
#define RXCS_WRITE_MASK   040137

enum DiskControllerAddresses {
    RXCS = 0777170,
    RXDB = 0777172
};

enum class RXCSFlag {
    GO   = 0000001,
    FS   = 0000016,
    US   = 0000020,
    DONE = 0000040,
    INT  = 0000100,
    XFER = 0000200
};

enum class DiskControllerState {
    BEGIN,
    SECTOR,
    TRACK,
    FILL,
    EMPTY,
    WRITE_SECTOR,
    READ_SECTOR,
    DONE
};

enum class DiskControllerFunction {
    FILL_BUFFER = 0,
    EMPTY_BUFFER,
    WRITE_SECTOR,
    READ_SECTOR,
    NOT_USED,
    READ_STATUS,
    WRITE_DEL,
    READ_ERR,
    IDLE
};


class DiskController : public IBusElement
{
    public:
        DiskController();
        ~DiskController();

        void send(enum BusMessageType, uint32_t, uint16_t);
        void recv(enum BusMessageType, uint32_t, uint16_t);
        uint16_t bus_id();
        void set_bus(Bus*);

        void insert_disk_media(char *);

        void execute();

        void dump();

    private:
        void process_bus_message(enum BusMessageType, uint32_t, uint16_t);

        void fill_buffer();
        void empty_buffer();
        void read_sector();
        void write_sector();

        void clear_buffer_register();

        void set_status_register(uint16_t);
        void set_transfer_flag();
        void clear_transfer_flag();
        bool is_transfer_flag_set();
        void set_done_flag();
        void clear_done_flag();
        bool is_done_flag_set();
        void clear_writeable_flags();
        void clear_all_flags();

        uint16_t m_RXCS;
        uint16_t m_RXDB;
        uint8_t m_internal_buffer[BUFFER_SIZE] = {};
        uint8_t m_buffer_index;
        uint32_t m_track;
        uint32_t m_sector;
        FILE *m_disk_media;
        enum DiskControllerState m_state;
        enum DiskControllerFunction m_function;

        Bus* m_bus_connection;
};

#endif