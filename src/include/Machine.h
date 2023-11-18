#ifndef MACHINE_H
#define MACHINE_H

#include <thread>

#include "Bus.h"
#include "CPU.h"
#include "Memory.h"
#include "DiskController.h"

class Machine
{
    public:
        Machine(char *);
        ~Machine();

        void init();
        void run();
        void halt();
        void dump_state();

    private:
        Bus m_bus;
        CPU m_cpu;
        Memory m_memory;
        DiskController m_disk;

        std::thread t_cpu;
        std::thread t_memory;
        std::thread t_disk;

        char *m_obj_file;
};

#endif
