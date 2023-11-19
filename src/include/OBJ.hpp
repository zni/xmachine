#ifndef OBJ_HPP
#define OBJ_HPP

#include <cstdio>
#include "Memory.hpp"

class OBJ
{
    public:
        OBJ();
        ~OBJ();

        void read(char *, Memory*);

    private:
        bool is_text_block(FILE*);
        void load_text_block(FILE*, Memory*);
};

#endif