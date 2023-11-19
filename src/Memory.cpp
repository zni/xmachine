#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include "include/Memory.h"

Memory::Memory()
{
    m_MAR = 0;
    m_MBR = 0;

    m_lower_addr = 0;
    m_upper_addr = MEMBYTES;
}

Memory::~Memory() {}

void Memory::send(enum BusMessage t, uint32_t addr, uint16_t data)
{
    m_bus_connection->send_bus_message(this, t, addr, data);
}

void Memory::recv(enum BusMessage t, uint32_t addr, uint16_t data)
{
    if (t == BusMessage::MSYN || t == BusMessage::SSYN) {
        return;
    }

    if (addr >= m_lower_addr && addr <= m_upper_addr) {
        process_message(t, addr, data);
    }
}

uint16_t Memory::bus_id()
{
    return 0000002;
}

void Memory::set_bus(Bus *b)
{
    m_bus_connection = b;
}

void Memory::write_word(uint32_t addr, uint16_t word)
{
    m_MAR = addr;
    m_MBR = word;
    m_MEMORY[m_MAR] = m_MBR & 0377;
    m_MEMORY[m_MAR + 1] = (m_MBR & 0177400) >> 8;
}

uint16_t Memory::read_word(uint32_t addr)
{
    uint16_t word = 0;
    word = m_MEMORY[addr];
    word |= m_MEMORY[addr + 1] << 8;
    return word;
}

void Memory::execute()
{
    while (!m_bus_connection->halted());
}

void Memory::dump()
{
    uint16_t row[16];
    bool all_zero = true;
    for (int r = 0; r < MEMWORDS; r += 32) {
        for (int c = 0, i = 0; c < 32; c += 2, i++) {
            if (read_word(r+c) != 0) { all_zero = false; }
            row[i] = read_word(r+c);
        }
        if (all_zero && ((r + 16) < MEMWORDS) && (r != 0)) {
            continue;
        }

        printf("0o%05o: ", r);
        for (int i = 0; i < 16; i++) {
            printf("0o%07o ", row[i]);
        }
        printf("\n");
        all_zero = true;
    }
}

void Memory::process_message(enum BusMessage t, uint32_t addr, uint16_t data)
{
    switch (t) {
        case BusMessage::DATI:
            send(BusMessage::SSYN, addr, read_word(addr));
            break;
        case BusMessage::DATIP:
            break;
        case BusMessage::DATO:
            m_MAR = addr;
            m_MBR = data;
            m_MEMORY[m_MAR] = m_MBR & 0377;
            m_MEMORY[m_MAR + 1] = (m_MBR & 0177400) >> 8;
            send(BusMessage::SSYN, m_MAR, m_MBR);
            break;
        case BusMessage::DATOB:
            m_MAR = addr;
            m_MBR = data;
            m_MEMORY[m_MAR] = m_MBR;
            send(BusMessage::SSYN, m_MAR, m_MBR);
            break;
        default:
            break;
    }
}
