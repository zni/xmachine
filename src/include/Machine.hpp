#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <thread>

#include "Bus.hpp"
#include "CPU.hpp"
#include "Memory.hpp"
#include "DiskController.hpp"

class Machine
{
    public:
        Machine(char *, bool single_step = false);
        ~Machine();

        void init();
        void run();
        void halt();
        void dump_state();
        void add_disk(char *);

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
