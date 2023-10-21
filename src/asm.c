#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc > 3) {
        fprintf(stderr, "usage: asm <source.mcode> <binary>");
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

    char line[8];
    uint16_t instruction;
    uint16_t program_size = 0;
    size_t len = 8;
    fwrite(&program_size, sizeof(program_size), 1, binary);
    while(!feof(source)) {
        for (int i = 0; i < len; i++) {
            line[i] = fgetc(source);
            if (line[i] == '\n' || line[i] == EOF) {
                line[i] = '\0';
                break;
            }
        }

        if (strncmp(line, ".end", 4) == 0) { 
            fseek(binary, 0, 0);
            fwrite(&program_size, sizeof(program_size), 1, binary);
            break; 
        }
        sscanf(line, "%ho", &instruction);
        fwrite(&instruction, sizeof(instruction), 1, binary);
        program_size++;
    }
    fclose(source);
    fclose(binary);

    return 0;
}