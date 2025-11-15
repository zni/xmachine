#ifndef LOAD_H
#define LOAD_H

#include <stdint.h>

#define HEADER_SIZE 16

typedef struct __attribute__((__packed__)) exec {
    uint16_t a_midmag;
    uint16_t a_text;
    uint16_t a_data;
    uint16_t a_bss;
    uint16_t a_syms;
    uint16_t a_entry;
    uint16_t a_trsize;
    uint16_t a_drsize;
} exec_t;

typedef struct __attribute__((__packed__)) relocation_info {
    uint16_t r_address;
    uint16_t r_symbolnum;
} rel_info_t;

typedef struct nlist {
    union {
        const char *n_name;
        long n_strx;
    } n_un;
    unsigned char n_type;
    char n_other;
    short n_desc;
    unsigned long n_value;
} nlist_t;

extern exec_t* aout_header_read(char *);
extern uint8_t* aout_text_read(char *, exec_t *);

#endif
