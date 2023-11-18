#ifndef MEMORY_H
#define MEMORY_H

#include "Bus.h"
#include "BusMessage.h"
#include "IBusElement.h"

#define MEMBYTES 65535
#define MEMWORDS 32767

class Memory : public IBusElement
{
    public:
        Memory();
        ~Memory();

        void send(enum BusMessageType, uint32_t, uint16_t);
        void recv(enum BusMessageType, uint32_t, uint16_t);
        uint16_t bus_id();
        void set_bus(Bus*);

        void write_word(uint32_t, uint16_t);
        uint16_t read_word(uint32_t);

        void execute();

        void dump();

    private:
        void process_message(enum BusMessageType, uint32_t, uint16_t);

        uint32_t m_MAR;
        uint16_t m_MBR;
        uint8_t m_MEMORY[MEMBYTES] = {};

        uint32_t m_lower_addr;
        uint32_t m_upper_addr;

        Bus* m_bus_connection;
};

#endif
