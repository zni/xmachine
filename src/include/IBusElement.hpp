#ifndef BUS_ELEMENT_HPP
#define BUS_ELEMENT_HPP

#include <cstdint>
#include "BusMessage.hpp"

class IBusElement
{
    public:
        virtual ~IBusElement() {}
        virtual void send(enum BusMessage, uint32_t, uint16_t) = 0;
        virtual void recv(enum BusMessage, uint32_t, uint16_t) = 0;
        virtual uint16_t bus_id() = 0;

        virtual void execute() = 0;
};

#endif
