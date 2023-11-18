#ifndef OBJ_H
#define OBJ_H

#include <cstdio>
#include "Memory.h"

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