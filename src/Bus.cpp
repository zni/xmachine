#include <iostream>
#include "include/Bus.h"

Bus::Bus() {}
Bus::~Bus() {}

void Bus::register_bus_element(IBusElement *element)
{
    this->m_bus_elements.push_back(element);
}

void Bus::send_bus_message(IBusElement *sender, BusMessage t, uint32_t addr, uint16_t data)
{
    if (t == BusMessage::CLEAR) {
        std::cout << "Bus::CLEAR" << std::endl;
        this->msyn = NULL;
        this->ssyn = NULL;
        return;
    }

    if (t == BusMessage::MSYN) {
        this->msyn = sender;
    } else if (t == BusMessage::SSYN) {
        this->ssyn = sender;
    }

    if (this->msyn == NULL || this->ssyn == NULL) {
        for (IBusElement *e : this->m_bus_elements) {
            if (e->bus_id() == sender->bus_id()) {
                continue;
            } else {
                e->recv(t, addr, data);
            }
        }
    } else {
        if (t == BusMessage::DATI) {
            this->ssyn->recv(t, addr, data);
        } else if (t == BusMessage::DATIP) {
            this->ssyn->recv(t, addr, data);
        } else if (t == BusMessage::DATO) {
            this->ssyn->recv(t, addr, data);
        } else if (t == BusMessage::DATOB) {
            this->ssyn->recv(t, addr, data);
        } else if (t == BusMessage::SSYN) {
            this->msyn->recv(t, addr, data);
        }
    }
}

void Bus::set_halt(bool halt)
{
    this->m_halted = halt;
}

bool Bus::halted()
{
    return this->m_halted;
}
