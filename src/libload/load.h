#ifndef LOAD_H
#define LOAD_H

#include <stdint.h>

typedef struct _aout_header {
    uint16_t a_midmag;
    uint16_t a_text;
    uint16_t a_data;
    uint16_t a_bss;
    uint16_t a_syms;
    uint16_t a_entry;
    uint16_t a_trsize;
    uint16_t a_drsize;
} aout_header;

typedef struct _symbol {
	uint16_t name[4];
	uint8_t flag;
	uint16_t value;
} symbol;

typedef struct _aout_object {
	aout_header header;
	uint16_t *text;
	/* symbol *table; */
} aout_object;


aout_object* aout_read(char *);
void aout_free(aout_object**);

#endif
