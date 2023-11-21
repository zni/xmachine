#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <ncurses.h>

#include "Bus.hpp"
#include "BusMessage.hpp"
#include "IBusElement.hpp"

#define MEMBYTES 65535
#define MEMWORDS 32767

class Memory : public IBusElement
{
    public:
        Memory();
        ~Memory();

        void send(enum BusMessage, uint32_t, uint16_t);
        void recv(enum BusMessage, uint32_t, uint16_t);
        uint16_t bus_id();
        void set_bus(Bus*);

        void write_word(uint32_t, uint16_t);
        uint16_t read_word(uint32_t);

        void execute();

        void set_window(WINDOW *);
        void dump();

    private:
        void process_message(enum BusMessage, uint32_t, uint16_t);

        uint32_t m_MAR;
        uint16_t m_MBR;
        uint8_t m_MEMORY[MEMBYTES] = {};

        uint32_t m_lower_addr;
        uint32_t m_upper_addr;

        Bus* m_bus_connection;
        WINDOW *m_window;
};

#endif
