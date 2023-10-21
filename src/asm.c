#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc > 3) {
        fprintf(stderr, "usage: asm <file.asm> <binary>");
        return 1;
    }

    char *source_filename = argv[1];
    char *binary_filename = argv[2];
    FILE *source = fopen(source_filename, "r");
    if (source == NULL) {
        fprintf(stderr, "Failed to open: %s\n", source_filename);
        return 1;
    }

    FILE *binary = fopen(binary_filename, "w");
    if (binary == NULL) {
        fprintf(stderr, "Failed to open: %s\n", binary_filename);
        return 1;
    }

    char *line;
    uint16_t instruction;
    size_t len = 8;
    while(!feof(source)) {
        line = fgetln(source, &len);
        line[7] = '\0';

        if (strncmp(line, ".end", 4) == 0) { break; }
        instruction = sscanf("%i", line);
        fwrite(&instruction, sizeof(instruction), 1, binary);
    }
    fclose(source);
    fclose(binary);

    return 0;
}