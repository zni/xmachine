#include <chrono>
#include <iostream>
#include <thread>

#include <getopt.h>
#include <signal.h>

#include "include/Machine.hpp"

Machine *machine = NULL;

void handle_user_interrupt(int signo)
{
    std::cout << std::endl;
    std::cout << "USER HALT..." << std::endl;
    machine->dump_state();
    machine->halt();
}

int main(int argc, char** argv)
{
    bool tty_enable = false;
    char *obj_file = NULL;
    char *disk_name = NULL;
    char opt;
    while ((opt = getopt(argc, argv, "d:o:")) != -1) {
        switch (opt) {
            case 'd':
                disk_name = optarg;
                break;
            case 'o':
                obj_file = optarg;
                break;
            default:
                break;
        }
    }

    if (obj_file == NULL) {
        std::cout << "usage: xmachine -o <OBJ file>" << std::endl;
        std::cout << "\t-d <disk image> : file to use with the disk controller" << std::endl;
        exit(EXIT_FAILURE);
    }

    Machine m(obj_file);
    machine = &m;

    if (disk_name != NULL) {
        m.add_disk(disk_name);
    }

    // Use the old style signal handler.
    signal(SIGINT, &handle_user_interrupt);

    m.init();
    m.run();

    return 0;
}
