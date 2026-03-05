#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../libload/load.h"

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

	char *obj_file = argv[1];

	aout_object *aout = aout_read(obj_file);
	if (aout == NULL) {
		printf("Failed to object.\n");
		exit(EXIT_FAILURE);
	}

	printf(".header\n");
	printf("midmag : %04o\n", aout->header.a_midmag);
	printf("text   : %d\n", aout->header.a_text);
	printf("data   : %d\n", aout->header.a_data);
	printf("bss    : %d\n", aout->header.a_bss);
	printf("syms   : %d\n", aout->header.a_syms);
	printf("entry  : %04o\n", aout->header.a_entry);
	printf("trsize : %d\n", aout->header.a_trsize);
	printf("drsize : %d\n", aout->header.a_drsize);

	printf("\n.text\n");
	int i;
	/* Why the shift? a_text is the count of bytes and I want words. */
	for (i = 0; i < (aout->header.a_text >> 1); i++) {
		printf("0o%06o\n", aout->text[i]);
	}

/*
	printf("\n.symbol_table\n");
	for (i = 0; i < (aout->header.a_syms >> 1); i++) {
		printf("0o%06o\n", aout->table[i].value);
	}
*/

	aout_free(&aout);

	return 0;
}

