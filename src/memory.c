#include <stdlib.h>
#include <string.h>
#include "include/memory.h"

uint16_t translate_register(uint16_t reg)
{
    switch (reg) {
        case 0:
            return R0;
        case 1:
            return R1;
        case 2:
            return R2;
        case 3:
            return R3;
        case 4:
            return R4;
        case 5:
            return R5;
        case 6:
            return R6;
        case 7:
            return R7;
        case 8:
        default:
            return R8;
    }
}

void write_word(memory_t *m, uint16_t word)
{
    if (m->dest % 2 != 0) { m->dest--; }
    m->_memory[m->dest] = word & 0377;
    m->_memory[m->dest + 1] = (word & 0177400) >> 8;
}

void write_byte(memory_t *m, uint8_t byte)
{
    m->_memory[m->dest] = byte;
}

uint16_t read_word(memory_t *m)
{
    uint16_t word = m->_memory[m->src] | (m->_memory[m->src + 1] << 8);
    return word;
}

uint8_t read_byte(memory_t *m)
{
    return m->_memory[m->src];
}

uint16_t direct_read_word(memory_t *m, uint16_t loc)
{
    uint16_t word = (m->_memory[loc] | (m->_memory[loc + 1] << 8));
    return word;
}

uint8_t direct_read_byte(memory_t *m, uint16_t loc)
{
    return m->_memory[loc];
}

void direct_write_word(memory_t *m, uint16_t loc, uint16_t v)
{
    m->_memory[loc] = v & 0377;
    m->_memory[loc + 1] = (v & 0177400) >> 8;
}

void direct_write_byte(memory_t *m, uint16_t loc, uint8_t v)
{
    m->_memory[loc] = v;
}

void word_advance(memory_t *m, uint16_t loc)
{
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void byte_advance(memory_t *m, uint16_t loc)
{
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;   
}

void word_decrease(memory_t *m, uint16_t loc)
{
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void byte_decrease(memory_t *m, uint16_t loc)
{
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void word_advance_r(memory_t *m, uint16_t r)
{
    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void byte_advance_r(memory_t *m, uint16_t r)
{
    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void word_decrease_r(memory_t *m, uint16_t r)
{
    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

void byte_decrease_r(memory_t *m, uint16_t r)
{
    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

// TODO probably split this out into a byte and word version.
void set_r(memory_t *m, uint16_t r, uint16_t val)
{
    uint16_t loc = translate_register(r);
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;
}

// TODO probably split this out into a byte and word version.
uint16_t get_r(memory_t *m, uint16_t r)
{
    uint16_t loc = translate_register(r);
    return m->_memory[loc] | (m->_memory[loc + 1] << 8);
}

void initialize_memory(memory_t **m)
{
    *m = malloc(sizeof(memory_t));
    (*m)->_memory = malloc(sizeof(uint8_t) * MEMBYTES);
    memset((*m)->_memory, 0, MEMBYTES);

    (*m)->write_word = &write_word;
    (*m)->write_byte = &write_byte;
    (*m)->read_word = &read_word;
    (*m)->read_byte = &read_byte;

    (*m)->direct_write_word = &direct_write_word;
    (*m)->direct_write_byte = &direct_write_byte;
    (*m)->direct_read_word = &direct_read_word;
    (*m)->direct_read_byte = &direct_read_byte;
    (*m)->word_advance = &word_advance;
    (*m)->byte_advance = &byte_advance;
    (*m)->word_decrease = &word_decrease;
    (*m)->byte_decrease = &byte_decrease;

    (*m)->word_advance_r = &word_advance_r;
    (*m)->byte_advance_r = &byte_advance_r;
    (*m)->word_decrease_r = &word_decrease_r;
    (*m)->byte_decrease_r = &byte_decrease_r;
    (*m)->set_r = &set_r;
    (*m)->get_r = &get_r;
}

void free_memory(memory_t **m)
{
    free((*m)->_memory);
    (*m)->_memory = NULL;
    free(*m);
    m = NULL;
}