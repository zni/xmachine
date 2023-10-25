#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc < 4) {
        fprintf(stderr, "usage: asm <source.mcode> <load offset> <binary>");
        return 1;
    }

    uint16_t load_offset = 0;
    if (sscanf(argv[2], "%hu", &load_offset) <= 0) {
        fprintf(stderr, "Failed to read load offset.");
        return 1;
    }

    char *source_filename = argv[1];
    char *binary_filename = argv[3];
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

    char peek;
    char line[8];
    uint16_t instruction = 0;
    uint16_t program_size = 0;
    size_t len = 8;
    fwrite(&program_size, sizeof(program_size), 1, binary);
    fwrite(&load_offset, sizeof(load_offset), 1, binary);
    while(!feof(source)) {
        peek = fgetc(source);
        if (peek == ';') {
            while (fgetc(source) != '\n');
            continue;
        } else if (peek == EOF) {
            break;
        } else {
            line[0] = peek;
        }

        for (int i = 1; i < len; i++) {
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
        } else {
            sscanf(line, "%ho", &instruction);
            printf("%07o - %hu\n", instruction, program_size);
            fwrite(&instruction, sizeof(instruction), 1, binary);
            program_size++;
        }
    }
    fclose(source);
    fclose(binary);

    return 0;
}