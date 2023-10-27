#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMSIZE 8192

#define R0 017400
#define R1 017402
#define R2 017404
#define R3 017406
#define R4 017410
#define R5 017412
#define R6 017414
#define R7 017416
#define R8 017420

#define R_PC 7
#define R_PS 8


typedef struct memory {
    uint16_t dest;
    void (*write_word)(void*, uint16_t);
    void (*write_byte)(void*, uint8_t);

    uint16_t src;
    uint16_t (*read_word)(void*);
    uint8_t  (*read_byte)(void*);

    uint16_t (*direct_read_word)(void*, uint16_t);
    uint8_t (*direct_read_byte)(void*, uint8_t);

    void (*direct_write_word)(void*, uint16_t, uint16_t);
    void (*direct_write_byte)(void*, uint16_t, uint8_t);

    void (*word_advance)(void*, uint16_t);
    void (*byte_advance)(void*, uint16_t);
    void (*word_decrease)(void*, uint16_t);
    void (*byte_decrease)(void*, uint16_t);

    void (*word_advance_r)(void*, uint16_t);
    void (*byte_advance_r)(void*, uint16_t);
    void (*word_decrease_r)(void*, uint16_t);
    void (*byte_decrease_r)(void*, uint16_t);
    void (*set_r)(void*, uint16_t, uint16_t);
    uint16_t (*get_r)(void*, uint16_t);

    uint8_t *_memory;
} memory_t;

uint16_t translate_register(uint16_t);

memory_t* initialize_memory();
void free_memory(memory_t**);

#endif