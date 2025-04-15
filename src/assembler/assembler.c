// chip8_assembler.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>


#define MAX_LABELS 512
#define MAX_LINE_LEN 128
#define ROM_START 0x200

typedef struct {
    char name[32];
    uint16_t address;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;

uint8_t rom[4096];
int rom_pos = 0;

uint16_t current_address = ROM_START;

void add_label(const char *name, uint16_t addr) {
    assert(label_count < MAX_LABELS);
    if(strlen(name) >= sizeof(labels[label_count].name)) {
        fprintf(stderr, "Label name too long: %s\n", name);
        exit(1);
    }
    strcpy(labels[label_count].name, name);
    labels[label_count].address = addr;
    label_count++;
}

int is_label(const char *token) {
    return token[strlen(token) - 1] == ':';
}

uint16_t resolve_label(const char *label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, label) == 0) return labels[i].address;
    }
    fprintf(stderr, "Unknown label: %s\n", label);
    exit(1);
}

uint8_t parse_register(const char *tok) {
    if (tok[0] != 'V') {
        fprintf(stderr, "Invalid register: %s\n", tok);
        exit(1);
    }
    return (uint8_t)strtol(tok + 1, NULL, 16);
}

uint8_t parse_imm(const char *tok) {
    if (tok[0] == '0' && tok[1] == 'x') {
        return (uint8_t)strtol(tok + 2, NULL, 16);
    }
    if (tok[0] == '0' && tok[1] == 'b') {
        return (uint8_t)strtol(tok + 2, NULL, 2);
    }
    return (uint8_t)strtol(tok, NULL, 0);
}

uint16_t parse_addr(const char *tok) {
    if (isalpha(tok[0])) return resolve_label(tok);
    return (uint16_t)strtol(tok, NULL, 0);
}

void emit(uint16_t instr) {
    rom[rom_pos++] = instr >> 8;
    rom[rom_pos++] = instr & 0xFF;
    current_address += 2;
}

void emit_byte(uint8_t byte) {
    rom[rom_pos++] = byte;
    current_address += 1;
}

void parse_instruction(char *line) {
    // Remove comments
    char *comment = strchr(line, ';');
    if (comment) *comment = '\0';

    char *tokens[4];
    int tokc = 0;
    char *tok = strtok(line, " ,\n");
    while (tok && tokc < 4) {
        tokens[tokc++] = tok;
        tok = strtok(NULL, " ,\n");
    }
    if (tokc == 0) return;
    if (is_label(tokens[0])) return;

    #define R(i) parse_register(tokens[i])
    #define N(i) parse_imm(tokens[i])
    #define A(i) parse_addr(tokens[i])
    #define T(mnem) (strcmp(tokens[0], mnem) == 0)
    #define is_reg(i) (tokens[i][0] == 'V')


    if (T("CLS")) emit(0x00E0);
    else if (T("RET")) emit(0x00EE);
    else if (T("JP")) emit(0x1000 | A(1));
    else if (T("CALL")) emit(0x2000 | A(1));
    else if (T("SE") && is_reg(1) && !is_reg(2)) emit(0x3000 | (R(1) << 8) | N(2));
    else if (T("SE") && is_reg(1) && is_reg(2)) emit(0x5000 | (R(1) << 8) | (R(2) << 4));
    else if (T("SNE") && is_reg(1) && !is_reg(2)) emit(0x4000 | (R(1) << 8) | N(2));
    else if (T("SNE") && is_reg(1) && is_reg(2)) emit(0x9000 | (R(1) << 8) | (R(2) << 4));
    else if (T("LD") && is_reg(1) && !is_reg(2)) emit(0x6000 | (R(1) << 8) | N(2));
    else if (T("LD") && is_reg(1) && is_reg(2)) emit(0x8000 | (R(1) << 8) | (R(2) << 4));
    else if (T("LD") && strcmp(tokens[1], "I") == 0) emit(0xA000 | A(2));
    else if (T("LD") && strcmp(tokens[1], "DT") == 0) emit(0xF015 | (R(2) << 8));
    else if (T("LD") && strcmp(tokens[1], "ST") == 0) emit(0xF018 | (R(2) << 8));
    else if (T("LD") && strcmp(tokens[2], "DT") == 0) emit(0xF007 | (R(1) << 8));
    else if (T("LD") && strcmp(tokens[2], "K") == 0) emit(0xF00A | (R(1) << 8));
    else if (T("LD") && strcmp(tokens[1], "F") == 0) emit(0xF029 | (R(2) << 8));
    else if (T("LD") && strcmp(tokens[1], "B") == 0) emit(0xF033 | (R(2) << 8));
    else if (T("LD") && tokens[2][0] == '[' && tokens[2][1] == 'I') emit(0xF055 | (R(1) << 8)); // LD [I], Vx
    else if (T("LD") && tokens[1][0] == '[' && tokens[1][1] == 'I') emit(0xF065 | (R(2) << 8)); // LD Vx, [I]
    else if (T("ADD") && is_reg(1)) emit(0x7000 | (R(1) << 8) | N(2));
    else if (T("ADD") && strcmp(tokens[1], "I") == 0) emit(0xF01E | (R(2) << 8));
    else if (T("OR")) emit(0x8001 | (R(1) << 8) | (R(2) << 4));
    else if (T("AND")) emit(0x8002 | (R(1) << 8) | (R(2) << 4));
    else if (T("XOR")) emit(0x8003 | (R(1) << 8) | (R(2) << 4));
    else if (T("SUB")) emit(0x8005 | (R(1) << 8) | (R(2) << 4));
    else if (T("SUBN")) emit(0x8007 | (R(1) << 8) | (R(2) << 4));
    else if (T("SHR")) emit(0x8006 | (R(1) << 8));
    else if (T("SHL")) emit(0x800E | (R(1) << 8));
    else if (T("RND")) emit(0xC000 | (R(1) << 8) | N(2));
    else if (T("SKP")) emit(0xE09E | (R(1) << 8));
    else if (T("SKNP")) emit(0xE0A1 | (R(1) << 8));
    else if (T("DRW")) emit(0xD000 | (R(1) << 8) | (R(2) << 4) | N(3));
    else if (T("DB")) emit_byte(N(1));
    else {
        fprintf(stderr, "Unknown instruction or operand format: %s\n", tokens[0]);
        exit(1);
    }
}

void first_pass(FILE *fp) {
    char line[MAX_LINE_LEN];
    uint16_t addr = ROM_START;
    while (fgets(line, sizeof(line), fp)) {
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';

        char *tok = strtok(line, " ,\n");
        if (!tok) continue;
        if (is_label(tok)) {
            tok[strlen(tok) - 1] = '\0';
            add_label(tok, addr);
        } else {
            if (strcmp(tok, "DB") == 0) addr += 1;
            else addr += 2;
        }
    }
    fseek(fp, 0, SEEK_SET);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source.asm> <output.rom>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("fopen");
        return 1;
    }

    first_pass(in);

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), in)) {
        parse_instruction(line);
    }
    fclose(in);

    FILE *out = fopen(argv[2], "wb");
    fwrite(rom, 1, rom_pos, out);
    fclose(out);

    return 0;
}