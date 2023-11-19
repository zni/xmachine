#include <iostream>

#include "include/Machine.hpp"
#include "include/OBJ.hpp"

Machine::Machine(char *obj_file, bool single_step)
{
    m_obj_file = obj_file;
    m_cpu.set_bus(&m_bus);
    m_cpu.set_step_mode(single_step);
    m_memory.set_bus(&m_bus);
    m_disk.set_bus(&m_bus);
    m_tty.set_bus(&m_bus);
}

Machine::~Machine()
{

}

void Machine::init()
{
    OBJ obj_reader;
    obj_reader.read(m_obj_file, &m_memory);

    m_bus.register_bus_element(&m_cpu);
    m_bus.register_bus_element(&m_memory);
    m_bus.register_bus_element(&m_disk);
    m_bus.register_bus_element(&m_tty);
    m_bus.set_halt(false);
}

void Machine::run()
{
    t_memory = std::thread(&Memory::execute, &m_memory);
    t_cpu = std::thread(&CPU::execute, &m_cpu);
    t_disk = std::thread(&DiskController::execute, &m_disk);
    t_tty = std::thread(&TTY::execute, &m_tty);
    t_memory.join();
    t_cpu.join();
    t_disk.join();
    t_tty.join();
    //dump_state();
}

void Machine::halt()
{
    m_bus.set_halt(true);
}

void Machine::dump_state()
{
    m_cpu.dump();
    m_memory.dump();
    m_disk.dump();
}

void Machine::add_disk(char *disk)
{
    m_disk.insert_disk_media(disk);
}