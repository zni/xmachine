#include <ncurses.h>
#include "include/TTY.hpp"

TTY::TTY()
{
    initscr();
}

TTY::~TTY()
{
    endwin();
}

void TTY::send(enum BusMessage t, uint32_t addr, uint16_t data)
{
    m_bus_connection->send_bus_message(this, t, addr, data);
}

void TTY::recv(enum BusMessage t, uint32_t addr, uint16_t data)
{
    if (is_internal_address(addr)) {
        process_bus_message(t, addr, data);
    }
}

uint16_t TTY::bus_id()
{

}

void TTY::set_bus(Bus *bus)
{
    m_bus_connection = bus;
}

void TTY::execute()
{

}

void TTY::process_bus_message(enum BusMessage t, uint32_t addr, uint16_t data)
{
    switch (t) {
        // Read request
        case BusMessage::DATI: {
            switch (addr) {
                case TKS:
                    send(BusMessage::SSYN, addr, m_TKS);
                    break;
                case TKB:
                    read_tkb_buffer();
                    send(BusMessage::SSYN, addr, m_TKB);
                    break;
                case TPS:
                    send(BusMessage::SSYN, addr, m_TPS);
                    break;
                case TPB:
                    send(BusMessage::SSYN, addr, m_TPB);
                    break;
            }
            break;
        }

        // Not currently implemented.
        case BusMessage::DATIP:
            break;

        // Write request.
        case BusMessage::DATO:{
            switch (addr) {
                case TKS:
                    set_tks_register(data);
                    send(BusMessage::SSYN, addr, m_TKS);
                    break;
                case TKB:
                    // Read-only
                    send(BusMessage::SSYN, addr, 0);
                    break;
                case TPS:
                    set_tps_register(data);
                    send(BusMessage::SSYN, addr, m_TPS);
                    break;
                case TPB:
                    set_tpb_buffer(data);
                    send(BusMessage::SSYN, addr, m_TPB);
                    break;
            }
            break;
        }

        // Write byte request.
        case BusMessage::DATOB:{
            switch (addr) {
                case TKS:
                    set_tks_register(data);
                    send(BusMessage::SSYN, addr, m_TKS);
                    break;
                case TKB:
                    // Read-only
                    send(BusMessage::SSYN, addr, 0);
                    break;
                case TPS:
                    set_tps_register(data);
                    send(BusMessage::SSYN, addr, m_TPS);
                    break;
                case TPB:
                    set_tpb_buffer(data);
                    send(BusMessage::SSYN, addr, m_TPB);
                    break;
            }
            break;
        }

        default:
            break;
    }
}

bool TTY::is_internal_address(uint32_t addr)
{
    switch (addr) {
        case TKS:
        case TKB:
        case TPS:
        case TPB:
            return true;
        default:
            return false;
    }
}

void TTY::set_tks_register(uint16_t data)
{
    m_TKS = data & TKS_WRITE_MASK;
}

void TTY::set_tks_busy_flag()
{

}

void TTY::clear_tks_mode_flags()
{

}

uint16_t TTY::read_tkb_buffer()
{
    clear_tks_mode_flags();
    set_tks_busy_flag();

}

void TTY::set_tps_register(uint16_t data)
{
    m_TPS = data & TPS_WRITE_MASK;
}

void TTY::set_tpb_buffer(uint16_t data)
{
    m_TPB = data & 0377;
}