#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define MEMBYTES 16384
#define MEMWORDS 8192

#define MAX_HANDLERS 20
#define INACTIVE_HANDLER_LOC 0xFFFF

#define R0 017400
#define R1 017402
#define R2 017404
#define R3 017406
#define R4 017410
#define R5 017412
#define R6 017414
#define R7 017416
#define R8 017420

#define R_SP 6
#define R_PC 7
#define R_PS 8

struct side_effect;
typedef struct side_effect side_effect_t;

struct memory;
typedef struct memory memory_t;
struct memory {
    uint16_t dest;
    void (*write_word)(memory_t*, uint16_t);
    void (*write_byte)(memory_t*, uint8_t);

    uint16_t src;
    uint16_t (*read_word)(memory_t*);
    uint8_t  (*read_byte)(memory_t*);

    uint16_t (*direct_read_word)(memory_t*, uint16_t);
    uint8_t (*direct_read_byte)(memory_t*, uint16_t);

    void (*direct_write_word)(memory_t*, uint16_t, uint16_t);
    void (*direct_write_byte)(memory_t*, uint16_t, uint8_t);

    void (*word_advance)(memory_t*, uint16_t);
    void (*byte_advance)(memory_t*, uint16_t);
    void (*word_decrease)(memory_t*, uint16_t);
    void (*byte_decrease)(memory_t*, uint16_t);

    void (*word_advance_r)(memory_t*, uint16_t);
    void (*byte_advance_r)(memory_t*, uint16_t);
    void (*word_decrease_r)(memory_t*, uint16_t);
    void (*byte_decrease_r)(memory_t*, uint16_t);
    void (*set_r)(memory_t*, uint16_t, uint16_t);
    uint16_t (*get_r)(memory_t*, uint16_t);

    bool (*register_read_side_effect)(memory_t*, uint16_t, void (*)(memory_t*));
    bool (*register_write_side_effect)(memory_t*, uint16_t, void (*)(memory_t*));
    side_effect_t *read_side_effects;
    side_effect_t *write_side_effects;

    uint8_t *_memory;
};

struct side_effect {
    uint16_t loc;
    void (*handler)(memory_t*);
};

uint16_t translate_register(uint16_t);

void initialize_memory(memory_t**);
void free_memory(memory_t**);

#endif