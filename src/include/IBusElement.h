#ifndef BUS_ELEMENT_H
#define BUS_ELEMENT_H

#include <cstdint>
#include "BusMessage.h"

class IBusElement
{
    public:
        virtual ~IBusElement() {}
        virtual void send(enum BusMessageType, uint32_t, uint16_t) = 0;
        virtual void recv(enum BusMessageType, uint32_t, uint16_t) = 0;
        virtual uint16_t bus_id() = 0;

        virtual void execute() = 0;
};

#endif
