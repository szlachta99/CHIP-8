#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>
#include <stddef.h>

#define MAX_OPCODE_LEN 64

typedef union {
    uint16_t raw;

    struct {
        uint16_t n : 4;
        uint16_t y : 4;
        uint16_t x : 4;
        uint16_t : 4;
    } nibbles;

    struct {
        uint16_t nn : 8;
        uint16_t x : 4;
        uint16_t : 4;
    } type6;

    struct {
        uint16_t nnn : 12;
        uint16_t : 4;
    } addr;

    struct {
        uint16_t : 12;
        uint16_t opcode : 4;
    };

} Instruction;

typedef enum {
    SUCCESS = 0,
    ERR_INVALID_OPCODE = -1,
    ERR_BUFFER_TOO_SMALL = -2
} Result;

Result disassemble(Instruction instruction, char *output, size_t output_size);
void disassemble_all(const uint8_t *rom, size_t rom_size, char *output, size_t output_size);

#endif