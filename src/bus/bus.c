#include "bus.h"

void register_bus_element()
{
}

void send_bus_message(/* BusMessage t,*/ uint32_t addr, uint16_t data)
{
//    if (t == BusMessage::CLEAR) {
//        this->msyn = NULL;
//        this->ssyn = NULL;
//        return;
//    }
//
//    if (t == BusMessage::MSYN) {
//        this->msyn = sender;
//    } else if (t == BusMessage::SSYN) {
//        this->ssyn = sender;
//    }
//
//    if (this->msyn == NULL || this->ssyn == NULL) {
//        for (IBusElement *e : this->m_bus_elements) {
//            if (e->bus_id() == sender->bus_id()) {
//                continue;
//            } else {
//                e->recv(t, addr, data);
//            }
//        }
//    } else {
//        if (t == BusMessage::DATI) {
//            this->ssyn->recv(t, addr, data);
//        } else if (t == BusMessage::DATIP) {
//            this->ssyn->recv(t, addr, data);
//        } else if (t == BusMessage::DATO) {
//            this->ssyn->recv(t, addr, data);
//        } else if (t == BusMessage::DATOB) {
//            this->ssyn->recv(t, addr, data);
//        } else if (t == BusMessage::SSYN) {
//            this->msyn->recv(t, addr, data);
//        }
//    }
}

void halt(uint8_t halt)
{

}

uint8_t halted()
{

}

int main(int argc, char **argv)
{
    return 0;
}

