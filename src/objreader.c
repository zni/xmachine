#include <stdint.h>
#include <stdio.h>
#include "include/objreader.h"

// Note to future self, "RT-11 Software Support Manual, File Formats"

/*
 * This file implements extremely basic reading of 
 * programs from RT-11 OBJ files.
 */

int is_text_block(FILE *obj)
{
    uint8_t byte01, byte02, data;
    int8_t chksum;
    uint16_t len;

    fread(&byte01, sizeof(uint8_t), 1, obj);
    fread(&byte02, sizeof(uint8_t), 1, obj);
    fread(&len, sizeof(uint16_t), 1, obj);
    fread(&byte01, sizeof(uint8_t), 1, obj);
    if (byte01 != 3) {
        for (int i = 0; i < len-5; i++) fread(&data, sizeof(uint8_t), 1, obj);
        fread(&chksum, sizeof(uint8_t), 1, obj);
        return 0;
    } else {
        fseek(obj, -5, SEEK_CUR);
        return 1;
    }
}

void load_text_block(machine_state_t *machine, FILE *obj)
{
    uint8_t byte01, byte02, tag, pad;
    uint16_t word, len, load_offset, loc;

    fread(&byte01, sizeof(uint8_t), 1, obj);            // Read 01 00 data block tag.
    fread(&byte02, sizeof(uint8_t), 1, obj);
    fread(&len, sizeof(uint16_t), 1, obj);              // Read length word.   
    fread(&tag, sizeof(uint8_t), 1, obj);                // Read block type byte.
    fread(&load_offset, sizeof(uint16_t), 1, obj);      // Read load address word.
    fread(&pad, sizeof(uint8_t), 1, obj);               // Skip past pad byte.
    len -= 8;                                           // Adjust the length.
    loc = load_offset;

    printf("data block: %03o %03o\n", byte01, byte02);
    printf("\tlen: %hu\n", len + 8);
    printf("\ttag: %03o\n", tag);

    machine->memory->set_r(machine->memory, R_PC, load_offset);
    for (int i = 0; i < len; i += 2, loc += 2) {
        fread(&byte01, sizeof(uint8_t), 1, obj);
        fread(&byte02, sizeof(uint8_t), 1, obj);
        word = (byte02 << 8) | byte01;
        printf("%07o: %07o: %03o %03o\n", loc, word, byte01, byte02);
        machine->memory->direct_write_word(machine->memory, loc, word);
    }
}

void load_program_from_obj(machine_state_t *machine, char* object_file)
{
    FILE *obj = fopen(object_file, "rb");

    // Skip past other data blocks.
    while(!is_text_block(obj) && !feof(obj));

    if (!feof(obj)) {
        load_text_block(machine, obj);
    }

    fclose(obj);
}