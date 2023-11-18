#include <chrono>
#include <iostream>
#include <thread>

#include <getopt.h>
#include <signal.h>

#include "include/Machine.h"

Machine *machine = NULL;

void handle_user_interrupt(int signo)
{
    std::cout << std::endl;
    std::cout << "USER HALT..." << std::endl;
    machine->halt();
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "usage: xmachine <OBJ file>" << std::endl;
        exit(EXIT_FAILURE);
    }

    Machine m(argv[1]);
    machine = &m;

    // Use the old style signal handler.
    signal(SIGINT, &handle_user_interrupt);

    m.init();
    m.run();

    return 0;
}
