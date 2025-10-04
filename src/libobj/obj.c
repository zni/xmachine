#include <stdint.h>
#include <stdio.h>
#include "obj.h"

// Note to future self, "RT-11 Software Support Manual, File Formats"

/*
 * This file implements extremely basic reading of
 * programs from RT-11 OBJ files.
 */

void read(char *filename)
{
    FILE *obj_file = fopen(filename, "r");

    while (!feof(obj_file)) {
        if (is_text_block(obj_file)) {
            load_text_block(obj_file);
        }
    }

    fclose(obj_file);
}

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
        fseek(obj, len - 5, SEEK_CUR);
        fread(&chksum, sizeof(int8_t), 1, obj);
        return 0;
    } else {
        fseek(obj, -5, SEEK_CUR);
        return 1;
    }
}

void load_text_block(FILE *obj)
{
    uint8_t byte01, byte02, tag, pad;
    int8_t chksum;
    uint16_t word, len, load_offset, loc;

    fread(&byte01, sizeof(uint8_t), 1, obj);            // Read 01 00 data block tag.
    fread(&byte02, sizeof(uint8_t), 1, obj);

#ifdef DEBUG_OBJ
    printf("data block: %03o %03o\n", byte01, byte02);
#endif

    fread(&len, sizeof(uint16_t), 1, obj);              // Read length word.

#ifdef DEBUG_OBJ
    printf("\tlen: %hu\n", len);
#endif

    fread(&tag, sizeof(uint8_t), 1, obj);                // Read block type byte.

#ifdef DEBUG_OBJ
    printf("\ttag: %03o\n", tag);
#endif

    fread(&load_offset, sizeof(uint16_t), 1, obj);      // Read load address word.
    fread(&pad, sizeof(uint8_t), 1, obj);               // Skip past pad byte.
    len -= 8;                                           // Adjust the length.

    //for (int i = 0; i < len; i += 2, m_loc += 2) {
    for (int i = 0; i < len; i += 2) {
        fread(&byte01, sizeof(uint8_t), 1, obj);
        fread(&byte02, sizeof(uint8_t), 1, obj);
        word = (byte02 << 8) | byte01;
        //m->write_word(m_loc, word);

#ifdef DEBUG_OBJ
        printf("%07o: %03o %03o\n", word, byte01, byte02);
#endif
    }
    fread(&chksum, sizeof(int8_t), 1, obj);
}

