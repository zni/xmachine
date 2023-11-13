#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/memory.h"
#include "include/tty.h"

mtx_t memory_subsystem_write_mtx;

void process_write_side_effects(memory_t*, uint16_t);
void process_read_side_effects(memory_t*, uint16_t);

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
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[m->dest] = word & 0377;
    m->_memory[m->dest + 1] = (word & 0177400) >> 8;
    process_write_side_effects(m, m->dest);

    mtx_unlock(&memory_subsystem_write_mtx);
}

void write_byte(memory_t *m, uint8_t byte)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[m->dest] = byte;
    process_write_side_effects(m, m->dest);

    mtx_unlock(&memory_subsystem_write_mtx);
}

uint16_t read_word(memory_t *m)
{
    uint16_t word = m->_memory[m->src] | (m->_memory[m->src + 1] << 8);
    process_read_side_effects(m, m->src);

    return word;


}

uint8_t read_byte(memory_t *m)
{

    uint8_t byte = m->_memory[m->src];
    process_read_side_effects(m, m->src);

    return byte;
}

uint16_t direct_read_word(memory_t *m, uint16_t loc)
{

    uint16_t word = (m->_memory[loc] | (m->_memory[loc + 1] << 8));
    process_read_side_effects(m, loc);

    return word;
}

uint8_t direct_read_byte(memory_t *m, uint16_t loc)
{

    uint8_t byte = m->_memory[loc];
    process_read_side_effects(m, loc);

    return byte;
}

uint16_t direct_read_word_n(memory_t *m, uint16_t loc)
{

    uint16_t word = (m->_memory[loc] | (m->_memory[loc + 1] << 8));

    return word;
}

uint8_t direct_read_byte_n(memory_t *m, uint16_t loc)
{

    uint8_t byte = m->_memory[loc];

    return byte;
}

void direct_write_word(memory_t *m, uint16_t loc, uint16_t v)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[loc] = v & 0377;
    m->_memory[loc + 1] = (v & 0177400) >> 8;
    process_write_side_effects(m, loc);

    mtx_unlock(&memory_subsystem_write_mtx);
}

void direct_write_byte(memory_t *m, uint16_t loc, uint8_t v)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[loc] = v;
    process_write_side_effects(m, loc);

    mtx_unlock(&memory_subsystem_write_mtx);
}

void direct_write_word_n(memory_t *m, uint16_t loc, uint16_t v)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[loc] = v & 0377;
    m->_memory[loc + 1] = (v & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void direct_write_byte_n(memory_t *m, uint16_t loc, uint8_t v)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[loc] = v;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void direct_write_word_OR(memory_t *m, uint16_t loc, uint16_t v)
{
    mtx_lock(&memory_subsystem_write_mtx);

    m->_memory[loc] |= v & 0377;
    m->_memory[loc + 1] |= (v & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void word_advance(memory_t *m, uint16_t loc)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void byte_advance(memory_t *m, uint16_t loc)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void word_decrease(memory_t *m, uint16_t loc)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void byte_decrease(memory_t *m, uint16_t loc)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void word_advance_r(memory_t *m, uint16_t r)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void byte_advance_r(memory_t *m, uint16_t r)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val += 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void word_decrease_r(memory_t *m, uint16_t r)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 2;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

void byte_decrease_r(memory_t *m, uint16_t r)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t loc = translate_register(r);
    uint16_t val = m->_memory[loc] | (m->_memory[loc + 1] << 8);
    val -= 1;
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

// TODO probably split this out into a byte and word version.
void set_r(memory_t *m, uint16_t r, uint16_t val)
{
    mtx_lock(&memory_subsystem_write_mtx);

    uint16_t loc = translate_register(r);
    m->_memory[loc] = val & 0377;
    m->_memory[loc + 1] = (val & 0177400) >> 8;

    mtx_unlock(&memory_subsystem_write_mtx);
}

// TODO probably split this out into a byte and word version.
uint16_t get_r(memory_t *m, uint16_t r)
{

    uint16_t loc = translate_register(r);
    uint16_t value = m->_memory[loc] | (m->_memory[loc + 1] << 8);

    return value;
}

bool register_read_side_effect(memory_t *m, uint16_t loc, void (*handler)(memory_t*))
{
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (m->read_side_effects[i].loc == INACTIVE_HANDLER_LOC) {
            m->read_side_effects[i].loc = loc;
            m->read_side_effects[i].handler = handler;
            return true;
        }
    }

    return false;
}

void process_read_side_effects(memory_t *m, uint16_t loc)
{
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (m->read_side_effects[i].loc == loc) {
            m->read_side_effects[i].handler(m);
            break;
        }
    }
}

bool register_write_side_effect(memory_t *m, uint16_t loc, void (*handler)(memory_t*))
{
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (m->write_side_effects[i].loc == INACTIVE_HANDLER_LOC) {
            m->write_side_effects[i].loc = loc;
            m->write_side_effects[i].handler = handler;
            return true;
        }
    }

    return false;
}

void process_write_side_effects(memory_t *m, uint16_t loc)
{
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (m->write_side_effects[i].loc == loc) {
            m->write_side_effects[i].handler(m);
            break;
        }
    }
}


void initialize_memory(memory_t **m)
{
    *m = malloc(sizeof(memory_t));
    (*m)->_memory = malloc(sizeof(uint8_t) * MEMBYTES);
    memset((*m)->_memory, 0, MEMBYTES);

    (*m)->read_side_effects = malloc(sizeof(side_effect_t) * MAX_HANDLERS);
    for (int i = 0; i < MAX_HANDLERS; i++) {
        (*m)->read_side_effects[i].handler = NULL;
        (*m)->read_side_effects[i].loc = INACTIVE_HANDLER_LOC;
    }
    (*m)->write_side_effects = malloc(sizeof(side_effect_t) * MAX_HANDLERS);
    for (int i = 0; i < MAX_HANDLERS; i++) {
        (*m)->write_side_effects[i].handler = NULL;
        (*m)->write_side_effects[i].loc = INACTIVE_HANDLER_LOC;
    }

    (*m)->register_read_side_effect = &register_read_side_effect;
    (*m)->register_write_side_effect = &register_write_side_effect;

    (*m)->write_word = &write_word;
    (*m)->write_byte = &write_byte;
    (*m)->read_word = &read_word;
    (*m)->read_byte = &read_byte;

    (*m)->direct_write_word = &direct_write_word;
    (*m)->direct_write_word_n = &direct_write_word_n;
    (*m)->direct_write_word_OR = &direct_write_word_OR;

    (*m)->direct_write_byte = &direct_write_byte;
    (*m)->direct_write_byte_n = &direct_write_byte_n;

    (*m)->direct_read_word = &direct_read_word;
    (*m)->direct_read_word_n = &direct_read_word_n;

    (*m)->direct_read_byte = &direct_read_byte;
    (*m)->direct_read_byte_n = &direct_read_byte_n;

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

    mtx_init(&memory_subsystem_write_mtx, mtx_plain|mtx_recursive);
}

void destroy_memory_mtx()
{
    mtx_destroy(&memory_subsystem_write_mtx);
}

void free_memory(memory_t **m)
{
    free((*m)->_memory);
    (*m)->_memory = NULL;

    free((*m)->read_side_effects);
    (*m)->read_side_effects = NULL;

    free((*m)->write_side_effects);
    (*m)->write_side_effects = NULL;

    free(*m);
    m = NULL;
}