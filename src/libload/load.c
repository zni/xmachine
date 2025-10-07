#include "load.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

exec_t*
aout_header_read(char *executable)
{
    int current_error;
    FILE *exe = fopen(executable, "r");
    if (exe == NULL) {
        current_error = errno;
        printf("Failed to read file: %s\n", strerror(current_error));
        return NULL;
    }

    exec_t *exec_data = malloc(sizeof(exec_t));
    if (exec_data == NULL) {
        current_error = errno;
        printf("Failed to allocate space: %s\n", strerror(current_error));
        return NULL;
    }

    if (fread(exec_data, sizeof(exec_t), 1, exe) == 0) {
        current_error = errno;
        printf("Failed to read data: %s\n", strerror(current_error));
        return NULL;
    }

    if (fclose(exe) != 0) {
        current_error = errno;
        printf("Failed to close file: %s\n", strerror(current_error));
        return NULL;
    }

    return exec_data;
}

uint8_t*
aout_text_read(char *executable, exec_t *header)
{
    int current_error;
    int fd = open(executable, O_RDONLY);
    if (fd == -1) {
        current_error = errno;
        printf("Failed to read file: %s\n", strerror(current_error));
        return NULL;
    }

    uint8_t *buffer = (uint8_t*) calloc(header->a_text, sizeof(uint8_t));
    if (buffer == NULL) {
        current_error = errno;
        printf("Failed to allocate buffer: %s\n", strerror(current_error));
        return NULL;
    }

    if (lseek(fd, HEADER_SIZE, SEEK_SET) == -1) {
        current_error = errno;
        printf("Failed to seek: %s\n", strerror(current_error));
        free(buffer);
        return NULL;
    }

    int i;
    for (i = 0; i < header->a_text; i++) {
        if (read(fd, &buffer[i], 1) == -1) {
            current_error = errno;
            printf("Failed to read: %s\n", strerror(current_error));
            free(buffer);
            return NULL;
        }
    }

    if (close(fd) != 0) {
        current_error = errno;
        printf("Failed to close file: %s\n", strerror(current_error));
        return NULL;
    }

    return buffer;
}

