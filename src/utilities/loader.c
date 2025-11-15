#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../src/libload/load.h"

void usage()
{
    printf("loader <file>\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }

    char *exe = argv[1];

    exec_t *header = aout_header_read(exe);
    if (header == NULL) {
        printf("Failed to read header.\n");
        exit(EXIT_FAILURE);
    }

    printf("midmag : %04o\n", header->a_midmag);
    printf("text   : %d\n", header->a_text);
    printf("data   : %d\n", header->a_data);
    printf("bss    : %d\n", header->a_bss);
    printf("syms   : %d\n", header->a_syms);
    printf("entry  : %04o\n", header->a_entry);
    printf("trsize : %d\n", header->a_trsize);
    printf("drsize : %d\n", header->a_drsize);

    uint8_t *text = aout_text_read(exe, header);
    if (text == NULL) {
        printf("Failed to read text.\n");
        exit(EXIT_FAILURE);
    }

    uint16_t dat = 0;
    int i = 0;
    for (i = 0; i < header->a_text; i++) {
        if ((i % 2) == 0) {
            dat = text[i];
        } else {
            dat |= (text[i] << 8) & 0xFF00;
            printf("%06o\n", dat);
            dat = 0;
        }
    }

    free(header);
    free(text);

    return 0;
}

