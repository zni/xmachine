#ifndef TTY_HPP
#define TTY_HPP

#include "Bus.hpp"
#include "IBusElement.hpp"

#define TKS_WRITE_MASK 0000101
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

enum class TKSFlag {
    RDRENB    = 0000001,
    INTERRUPT = 0000100,
    DONE      = 0000200,
    BUSY      = 0004000
};

enum class TPSFlag {
    INTERRUPT = 0000100,
    READY     = 0000200
};

class TTY : public IBusElement
{
    public:
        TTY();
        ~TTY();

        void send(enum BusMessage, uint32_t, uint16_t);
        void recv(enum BusMessage, uint32_t, uint16_t);
        uint16_t bus_id();
        void set_bus(Bus*);

        void execute();

    private:
        void process_bus_message(enum BusMessage, uint32_t, uint16_t);
        bool is_internal_address(uint32_t);

        void set_tks_register(uint16_t);
        void set_tks_busy_flag();
        void clear_tks_mode_flags();
        uint16_t read_tkb_buffer();

        void set_tps_register(uint16_t);
        void set_tpb_buffer(uint16_t);


        uint16_t m_TKS;
        uint16_t m_TKB;
        uint16_t m_TPS;
        uint16_t m_TPB;

        Bus *m_bus_connection;
};

#endif