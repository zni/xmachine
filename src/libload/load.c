#include "load.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

exec_t*
aout_header_read(char *executable)
{
    FILE *exe = fopen(executable, "r");
    if (exe == NULL) {
        perror("header fopen");
        return NULL;
    }

    exec_t *exec_data = malloc(sizeof(exec_t));
    if (exec_data == NULL) {
        perror("header malloc");
        return NULL;
    }

    if (fread(exec_data, sizeof(exec_t), 1, exe) == 0) {
        perror("header fread");
        free(exec_data);
        return NULL;
    }

    if (fclose(exe) != 0) {
        perror("header fclose");
        free(exec_data);
        return NULL;
    }

    return exec_data;
}

uint8_t*
aout_text_read(char *executable, exec_t *header)
{
    int fd = open(executable, O_RDONLY);
    if (fd == -1) {
        perror("text open");
        return NULL;
    }

    uint8_t *buffer = (uint8_t*) calloc(header->a_text, sizeof(uint8_t));
    if (buffer == NULL) {
        perror("text calloc");
        return NULL;
    }

    if (lseek(fd, HEADER_SIZE, SEEK_SET) == -1) {
        perror("text lseek");
        free(buffer);
        return NULL;
    }

    int i;
    for (i = 0; i < header->a_text; i++) {
        if (read(fd, &buffer[i], 1) == -1) {
            perror("text read");
            free(buffer);
            return NULL;
        }
    }

    if (close(fd) != 0) {
        perror("text close");
        free(buffer);
        return NULL;
    }

    return buffer;
}

