#include "disassembler.h"
#include <string.h>
#include <stdio.h>

Result disassemble(Instruction instruction, char* buffer, size_t buffer_size)
{
    switch (instruction.opcode) {
        case 0x0: 
            switch (instruction.raw)
            {
            case 0x00E0: // CLS
                snprintf(buffer, buffer_size, "CLS");
                return SUCCESS;
            case 0x00EE: // RET
                snprintf(buffer, buffer_size, "RET");
                return SUCCESS;    
            default:
                return ERR_INVALID_OPCODE;
            }            
        case 0x1: // JMP
            snprintf(buffer, buffer_size, "JMP 0x%03X", instruction.addr.nnn);
            return SUCCESS;
        case 0x2: // CALL
            snprintf(buffer, buffer_size, "CALL 0x%03X", instruction.addr.nnn);
            return SUCCESS;
        case 0x3: // SE VX
            snprintf(buffer, buffer_size, "SE V%X, 0x%02X", instruction.type6.x, instruction.type6.nn);
            return SUCCESS;
        case 0x4: // SNE VX
            snprintf(buffer, buffer_size, "SNE V%X, 0x%02X", instruction.type6.x, instruction.type6.nn);
            return SUCCESS;
        case 0x5: // SE VX VY
            snprintf(buffer, buffer_size, "SE V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
            return SUCCESS;
        case 0x6: // SET VX
            snprintf(buffer, buffer_size, "SET V%X, 0x%02X", instruction.type6.x, instruction.type6.nn);
            return SUCCESS;
        case 0x7: // ADD VX
            snprintf(buffer, buffer_size, "ADD V%X, 0x%02X", instruction.type6.x, instruction.type6.nn);
            return SUCCESS;
        case 0x8: // Arithmetic operations
            switch (instruction.nibbles.n)
            {
            case 0x0: // SET VX VY
                snprintf(buffer, buffer_size, "SET V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x1: // OR VX VY
                snprintf(buffer, buffer_size, "OR V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x2: // AND VX VY
                snprintf(buffer, buffer_size, "AND V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x3: // XOR VX VY
                snprintf(buffer, buffer_size, "XOR V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x4: // ADD VX VY
                snprintf(buffer, buffer_size, "ADD V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x5: // SUB VX VY
                snprintf(buffer, buffer_size, "SUB V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0x6: // SHR VX
                snprintf(buffer, buffer_size, "SHR V%X", instruction.nibbles.x);
                return SUCCESS;
            case 0x7: // SUBN VX VY
                snprintf(buffer, buffer_size, "SUBN V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
                return SUCCESS;
            case 0xE: // SHL VX
                snprintf(buffer, buffer_size, "SHL V%X", instruction.nibbles.x);
                return SUCCESS;
            default:
                return ERR_INVALID_OPCODE;
            }
        case 0x9: // SNE VX VY
            snprintf(buffer, buffer_size, "SNE V%X, V%X", instruction.nibbles.x, instruction.nibbles.y);
            return SUCCESS;
        case 0xA: // SET I
            snprintf(buffer, buffer_size, "SET I, 0x%03X", instruction.addr.nnn);
            return SUCCESS;
        case 0xB: // JMP VX
            snprintf(buffer, buffer_size, "JMP V0, 0x%03X", instruction.addr.nnn);
            return SUCCESS;
        case 0xC: // RND
            snprintf(buffer, buffer_size, "RND V%X, 0x%02X", instruction.type6.x, instruction.type6.nn);
            return SUCCESS;
        case 0xE: // Key operations
            switch (instruction.type6.nn)
            {
            case 0x9E: // SKP VX
                snprintf(buffer, buffer_size, "SKP V%X", instruction.type6.x);
                return SUCCESS;
            case 0xA1: // SKNP VX
                snprintf(buffer, buffer_size, "SKNP V%X", instruction.type6.x);
                return SUCCESS;
            default:
                return ERR_INVALID_OPCODE;
            }
        case 0xD: // Draw
            snprintf(buffer, buffer_size, "DRW V%X, V%X, 0x%02X", instruction.nibbles.x, instruction.nibbles.y, instruction.nibbles.n);
            return SUCCESS;
        case 0xF:
            switch (instruction.type6.nn)
            {
            case 0x07: // LD VX DT
                snprintf(buffer, buffer_size, "LD V%X, DT", instruction.type6.x);
                return SUCCESS;
            case 0x0A: // LD VX, K
                snprintf(buffer, buffer_size, "LD V%X, K", instruction.type6.x);
                return SUCCESS;
            case 0x15: // LD DT, VX
                snprintf(buffer, buffer_size, "LD DT, V%X", instruction.type6.x);
                return SUCCESS;
            case 0x18: // LD ST, VX
                snprintf(buffer, buffer_size, "LD ST, V%X", instruction.type6.x);
                return SUCCESS;
            case 0x1E: // ADD I, VX
                snprintf(buffer, buffer_size, "ADD I, V%X", instruction.type6.x);
                return SUCCESS;
            case 0x29: // LD F, VX
                snprintf(buffer, buffer_size, "LD F, V%X", instruction.type6.x);
                return SUCCESS;
            case 0x33: // LD BCD, VX
                snprintf(buffer, buffer_size, "LD BCD, V%X", instruction.type6.x);
                return SUCCESS;
            case 0x55: // LD [I], VX
                snprintf(buffer, buffer_size, "LD [I], V%X", instruction.type6.x);
                return SUCCESS;
            case 0x65: // LD VX, [I]
                snprintf(buffer, buffer_size, "LD V%X, [I]", instruction.type6.x);
                return SUCCESS;
            default:
                return ERR_INVALID_OPCODE;
            }
        default:
            return ERR_INVALID_OPCODE;
    }
}

void disassemble_all(const uint8_t *rom, size_t rom_size, char *output, size_t output_size)
{
    size_t offset = 0;
    size_t output_offset = 0;

    while (offset < rom_size) {
        if (output_offset >= output_size - MAX_OPCODE_LEN) {
            break; // Prevent buffer overflow
        }

        Instruction instruction;
        instruction.raw = (rom[offset] << 8) | rom[offset + 1];

        char opcode_str[MAX_OPCODE_LEN];
        Result result = disassemble(instruction, opcode_str, sizeof(opcode_str));

        if (result == SUCCESS) {
            snprintf(output + output_offset, output_size - output_offset, "0x%04zX:\t%s\n", offset, opcode_str);
            output_offset += strlen(output + output_offset);
        } else {
            snprintf(output + output_offset, output_size - output_offset, "0x%04zX:\tDB 0x%04X\n", offset, instruction.raw);
            output_offset += strlen(output + output_offset);
        }

        offset += 2;
    }
}
