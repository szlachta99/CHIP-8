#include "disassembler.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ROM file> <output file>\n", argv[0]);
        return 1;
    }
    FILE *rom_file = fopen(argv[1], "rb");
    if (!rom_file) {
        perror("Failed to open ROM file");
        return 1;
    }
    fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);
    uint8_t *rom = malloc(4096);
    if (!rom) {
        perror("Failed to allocate memory for ROM");
        fclose(rom_file);
        return 1;
    }
    fread(rom, 1, rom_size, rom_file);

    fclose(rom_file);
    char *output = malloc(rom_size * 16); // Rough estimate for output size
    if (!output) {
        perror("Failed to allocate memory for output");
        free(rom);
        return 1;
    }
    disassemble_all(rom, rom_size, output, rom_size * 16);
    free(rom);
    FILE *output_file = fopen(argv[2], "w");
    if (!output_file) {
        perror("Failed to open output file");
        free(output);
        return 1;
    }
    fputs(output, output_file);
    fclose(output_file);
    free(output);
    return 0;
}