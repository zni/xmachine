#ifndef BUS_HPP
#define BUS_HPP

#include <cstdint>
#include <list>

#include "BusMessage.hpp"
#include "IBusElement.hpp"

class Bus
{
    public:
        Bus();
        ~Bus();

        void register_bus_element(IBusElement*);
        void send_bus_message(IBusElement*, BusMessage, uint32_t, uint16_t);
        void set_halt(bool);
        bool halted();

    private:
        std::list<IBusElement*> m_bus_elements;

        bool m_halted = true;
        IBusElement *msyn = NULL;
        IBusElement *ssyn = NULL;
};

#endif
