#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "load.h"

#define HEADER_FIELDS 8
#define HEADER_SIZE 16
#define SYM_WORDS 6

static int aout_read_header(int fd, aout_object *aout);
static int aout_read_text(int fd, aout_object *aout);
/* static int aout_read_symbols(int fd, aout_object *aout); */

aout_object*
aout_read(char *executable)
{
	int fd = open(executable, O_RDONLY);
	if (fd == -1) {
		perror("aout_read open");
		return NULL;
	}

	aout_object *aout = (aout_object*) malloc(sizeof(aout_object));
	if (aout == NULL) {
		perror("aout_read malloc");
		return NULL;
	}

	aout->text = NULL;
	/* aout->table = NULL; */

	if (aout_read_header(fd, aout) != 0) {
		aout_free(&aout);
		close(fd);
		return NULL;
	}

	if (aout_read_text(fd, aout) != 0) {
		aout_free(&aout);
		close(fd);
		return NULL;
	}

/* Need to check out how the symbol table is actually laid out.
	if (aout_read_symbols(fd, aout) != 0) {
		aout_free(&aout);
		close(fd);
		return NULL;
	}
*/

	if (close(fd) != 0) {
		perror("aout_read close");
		aout_free(&aout);
		return NULL;
	}

	return aout;
}

int
aout_read_header(int fd, aout_object *aout)
{
	uint16_t* header_refs[HEADER_FIELDS] = {
		&(aout->header.a_midmag),
		&(aout->header.a_text),
		&(aout->header.a_data),
		&(aout->header.a_bss),
		&(aout->header.a_syms),
		&(aout->header.a_entry),
		&(aout->header.a_trsize),
		&(aout->header.a_drsize)
	};

	int i;
	for (i = 0; i < HEADER_FIELDS; i++) {
		if (read(fd, header_refs[i], sizeof(uint16_t)) == -1) {
			return -1;
		}
	}

	return 0;
}

int
aout_read_text(int fd, aout_object *aout)
{
	int n_text = aout->header.a_text >> 1;
	aout->text = (uint16_t*) calloc(n_text, sizeof(uint16_t));
	if (aout->text == NULL) {
		return -1;
	}

	int i;
	for (i = 0; i < n_text; i++) {
		if (read(fd, &(aout->text[i]), sizeof(uint16_t)) == -1) {
			return -1;
		}
	}

	return 0;
}

/*
int
aout_read_symbols(int fd, aout_object *aout)
{
	int n_syms = aout->header.a_syms >> 1;
	aout->table = (symbol*) calloc(n_syms, sizeof(symbol));
	if (aout->table == NULL) {
		return -1;
	}

	int i;
	for (i = 0; i < n_syms; i++) {
		if (read(fd, &(aout->table[i].name), sizeof(uint16_t) * 4) == -1) {
			return -1;
		}

		if (read(fd, &(aout->table[i].flag), sizeof(uint8_t)) == -1) {
			return -1;
		}

		if (read(fd, &(aout->table[i].value), sizeof(uint16_t)) == -1) {
			return -1;
		}
	}

	return 0;
}
*/

void aout_free(aout_object **aout)
{
	if ((*aout)->text != NULL) {
		free((*aout)->text);
		(*aout)->text = NULL;
	}

/* Not used yet.
	if ((*aout)->table != NULL) {
		free((*aout)->table);
	}
*/

	free(*aout);
	(*aout) = NULL;
}

