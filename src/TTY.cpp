#include <chrono>
#include <thread>
#include <ncurses.h>
#include "include/TTY.hpp"

TTY::TTY()
{
    m_TPS = static_cast<uint16_t>(TPSFlag::READY);
    m_TPB = 0;
    m_TKB = 0;
    m_TKS = 0;
    current_col = 0;
    current_row = 0;
    max_rows = 0;
    max_cols = 0;
}

TTY::~TTY()
{

}

void TTY::init_tty()
{
    getmaxyx(stdscr, max_rows, max_cols);
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
    return 0000004;
}

void TTY::set_bus(Bus *bus)
{
    m_bus_connection = bus;
}

void TTY::execute()
{
    init_tty();

    while (!m_bus_connection->halted()) {
        if (is_tks_busy()) {
            read_kb();
        }

        if (!is_tps_ready()) {
            write_char();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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
                    send(BusMessage::SSYN, addr, read_tkb_buffer());
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

void TTY::read_kb()
{
    m_TKB = getch();
    clear_tks_busy_flag();
    set_tks_done_flag();
}

void TTY::write_char()
{
    char c = m_TPB & 0377;
    if (c == '\n') {
        current_row++;
        current_col = 0;
        move(current_row, current_col);
        refresh();
        clear_tpb_buffer();
        set_tps_ready_flag();
        return;
    } else if (c == '\0') {
        refresh();
        clear_tpb_buffer();
        set_tps_ready_flag();
        return;
    }

    if (current_row == max_rows) {
        current_row = 0;
        current_col = 0;
        clear();
        refresh();
    }

    if (current_col == max_cols) {
        current_row++;
        current_col = 0;
    }

    mvaddch(current_row, current_col, m_TPB & 0377);
    current_col++;

    refresh();
    clear_tpb_buffer();
    set_tps_ready_flag();
}

void TTY::set_tks_register(uint16_t data)
{
    m_TKS = data & TKS_WRITE_MASK;

    if (m_TKS & static_cast<uint16_t>(TKSFlag::RDRENB)) {
        set_tks_busy_flag();
        clear_tks_rdrenb_flag();
    }
}

bool TTY::is_tks_busy()
{
    if (m_TKS & static_cast<uint16_t>(TKSFlag::BUSY)) {
        return true;
    } else {
        return false;
    }
}

void TTY::set_tks_busy_flag()
{
    m_TKS |= static_cast<uint16_t>(TKSFlag::BUSY);
}

void TTY::set_tks_done_flag()
{
    m_TKS |= static_cast<uint16_t>(TKSFlag::DONE);
}

void TTY::clear_tks_rdrenb_flag()
{
    m_TKS &= ~static_cast<uint16_t>(TKSFlag::RDRENB);
}

void TTY::clear_tks_busy_flag()
{
    m_TKS &= ~static_cast<uint16_t>(TKSFlag::BUSY);
}

void TTY::clear_tks_done_flag()
{
    m_TKS &= ~static_cast<uint16_t>(TKSFlag::DONE);
}

void TTY::clear_tks_mode_flags()
{
    m_TKS &= ~TKS_MODE_MASK;
}

uint16_t TTY::read_tkb_buffer()
{
    clear_tks_mode_flags();
    return m_TKB;
}

void TTY::set_tps_register(uint16_t data)
{
    m_TPS = data & TPS_WRITE_MASK;
}

bool TTY::is_tps_ready()
{
    if (m_TPS & static_cast<uint16_t>(TPSFlag::READY)) {
        return true;
    } else {
        return false;
    }
}

void TTY::set_tps_ready_flag()
{
    m_TPS |= static_cast<uint16_t>(TPSFlag::READY);
}

void TTY::clear_tps_ready_flag()
{
    m_TPS &= ~static_cast<uint16_t>(TPSFlag::READY);
}

void TTY::set_tpb_buffer(uint16_t data)
{
    m_TPB = data & 0377;
    clear_tps_ready_flag();
}

void TTY::clear_tpb_buffer()
{
    m_TPB = 0;
}