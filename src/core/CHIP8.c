#include "CHIP8.h"
#include <stdio.h>
#include <stdlib.h>

Instruction FetchInstruction(CHIP8 *chip8)
{
    chip8->program_counter += 2;
    return (Instruction) {
        .raw = (chip8->ram[chip8->program_counter - 2] << 8) | chip8->ram[chip8->program_counter - 1]
    };
}

void ExecuteInstruction(CHIP8 *chip8, Instruction instruction)
{
    switch (instruction.opcode) {
        case 0x0: 
            switch (instruction.raw)
            {
            case 0x00E0: // CLS
                for (int i = 0; i < 32; i++) {
                    chip8->screen[i] = 0;
                }
                chip8->screen_changed = 1;
                break;
                case 0x00EE: // RET
                chip8->program_counter = chip8->stack[--chip8->stack_pointer];
                break;    
            default:
                break;
            }            
            break;
        case 0x1: // JMP
            chip8->program_counter = instruction.addr.nnn;
            break;
        case 0x2: // CALL
            chip8->stack[chip8->stack_pointer++] = chip8->program_counter;
            chip8->program_counter = instruction.addr.nnn;
            break;
        case 0x3: // SE VX
            if (chip8->registers[instruction.type6.x] == instruction.type6.nn) {
                chip8->program_counter += 2;
            }
            break;
        case 0x4: // SNE VX
            if (chip8->registers[instruction.type6.x] != instruction.type6.nn) {
                chip8->program_counter += 2;
            }
            break;
        case 0x5: // SE VX VY
            if(chip8->registers[instruction.nibbles.x] == chip8->registers[instruction.nibbles.y]) {
                chip8->program_counter += 2;
            }
            break;
        case 0x6: // SET VX
            chip8->registers[instruction.type6.x] = instruction.type6.nn;
            break;
        case 0x7: // ADD VX
            chip8->registers[instruction.type6.x] += instruction.type6.nn;
            break;
        case 0x8: // Arithmetic operations
            switch (instruction.nibbles.n)
            {
            case 0x0: // SET VX VY
                chip8->registers[instruction.nibbles.x] = chip8->registers[instruction.nibbles.y];
                break;
            case 0x1: // OR VX VY
                chip8->registers[instruction.nibbles.x] |= chip8->registers[instruction.nibbles.y];
                break;
            case 0x2: // AND VX VY
                chip8->registers[instruction.nibbles.x] &= chip8->registers[instruction.nibbles.y];
                break;
            case 0x3: // XOR VX VY
                chip8->registers[instruction.nibbles.x] ^= chip8->registers[instruction.nibbles.y];
                break;
            case 0x4: // ADD VX VY
                chip8->registers[0xF] = (chip8->registers[instruction.nibbles.x] + chip8->registers[instruction.nibbles.y]) > 0xFF;
                chip8->registers[instruction.nibbles.x] += chip8->registers[instruction.nibbles.y];
                break;
            case 0x5: // SUB VX VY
                chip8->registers[0xF] = chip8->registers[instruction.nibbles.x] > chip8->registers[instruction.nibbles.y];
                chip8->registers[instruction.nibbles.x] -= chip8->registers[instruction.nibbles.y];
                break;
            case 0x6: // SHR VX
                chip8->registers[0xF] = chip8->registers[instruction.nibbles.x] & 0x1;
                chip8->registers[instruction.nibbles.x] >>= 1;
                break;
            case 0x7: // SUBN VX VY
                chip8->registers[0xF] = chip8->registers[instruction.nibbles.y] > chip8->registers[instruction.nibbles.x];
                chip8->registers[instruction.nibbles.x] = chip8->registers[instruction.nibbles.y] - chip8->registers[instruction.nibbles.x];
                break;
            case 0xE: // SHL VX
                chip8->registers[0xF] = (chip8->registers[instruction.nibbles.x] & 0x80) >> 7;
                chip8->registers[instruction.nibbles.x] <<= 1;
                break;
            default:
                break;
            }
            break;
        case 0x9: // SNE VX VY
            if(chip8->registers[instruction.nibbles.x] != chip8->registers[instruction.nibbles.y]) {
                chip8->program_counter += 2;
            }
            break;
        
        case 0xA: // SET I
            chip8->index = instruction.addr.nnn;
            break;
        case 0xB: // JMP VX
            chip8->program_counter = instruction.addr.nnn + chip8->registers[instruction.nibbles.x];
            break;
        case 0xC: // RND
            chip8->registers[instruction.type6.x] = (rand() % 256) & instruction.type6.nn;
            break;
        case 0xE: // Key operations
            switch (instruction.type6.nn)
            {
            case 0x9E: // SKP VX
                if(chip8->keypad[chip8->registers[instruction.type6.x] & 0xF]) {
                    chip8->program_counter += 2;
                }
                break;
            case 0xA1: // SKNP VX
                if(!chip8->keypad[chip8->registers[instruction.type6.x] & 0xF]) {
                    chip8->program_counter += 2;
                }
                break;
            default:
                break;
            }
            break;
        case 0xD: { // Draw
            uint8_t x = chip8->registers[instruction.nibbles.x] % 64;
            uint8_t y = chip8->registers[instruction.nibbles.y] % 32;

            uint8_t height = instruction.nibbles.n;
            
            chip8->registers[0xF] = 0;

            for(uint8_t row = 0; row < height; row++) {
                uint8_t real_row = (y + row) % 32;
                if(real_row >= 32) {
                    break; // Prevent out of bounds access
                }
                uint8_t sprite_raw = chip8->ram[chip8->index + row];

                uint64_t sprite_aligned = (uint64_t)sprite_raw << (56 - x);
                
                if(sprite_aligned & chip8->screen[real_row]) {
                    chip8->registers[0xF] = 1; // Collision detected
                }

                chip8->screen[real_row] ^= sprite_aligned;
            }

            chip8->screen_changed = 1;

            break;
        }
        case 0xF:
            switch (instruction.type6.nn)
            {
            case 0x07: // LD VX DT
                chip8->registers[instruction.type6.x] = chip8->delay_timer;
                break;
            case 0x0A: { // LD VX, K
                int key_found = -1;
                for (int i = 0; i < 16; i++) {
                    if (chip8->keypad[i] && !chip8->prev_keypad[i]) {
                        key_found = i;
                        break;
                    }
                }
            
                if (key_found >= 0) {
                    chip8->registers[instruction.type6.x] = key_found;
                } else {
                    chip8->program_counter -= 2; // Wait for key press
                }
                break;
            }
            case 0x15: // LD DT, VX
                chip8->delay_timer = chip8->registers[instruction.type6.x];
                break;
            case 0x18: // LD ST, VX
                chip8->sound_timer = chip8->registers[instruction.type6.x];
                break;
            case 0x1E: // ADD I, VX
                chip8->index += chip8->registers[instruction.type6.x];
                break;
            case 0x29: // LD F, VX
                chip8->index = chip8->registers[instruction.type6.x] * 5; // Font sprite location
                break;
            case 0x33: // LD BCD, VX
                chip8->ram[chip8->index] = chip8->registers[instruction.type6.x] / 100;
                chip8->ram[chip8->index + 1] = (chip8->registers[instruction.type6.x] / 10) % 10;
                chip8->ram[chip8->index + 2] = chip8->registers[instruction.type6.x] % 10;
                break;
            case 0x55: // LD [I], VX
                for (int i = 0; i <= instruction.type6.x; i++) {
                    chip8->ram[chip8->index + i] = chip8->registers[i];
                }
                break;
            case 0x65: // LD VX, [I]
                for (int i = 0; i <= instruction.type6.x; i++) {
                    chip8->registers[i] = chip8->ram[chip8->index + i];
                }
                break;
            
            default:
                break;
            }
        default:
            break;
    }
}

void InitializeCHIP8(CHIP8 *chip8)
{
    chip8->program_counter = CHIP8_ROM_ADDR; // Initialize program counter to start of ROM
    chip8->index = 0x0;
    chip8->stack_pointer = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->screen_changed = 1;
    for (int i = 0; i < 16; i++) {
        chip8->registers[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        chip8->screen[i] = 0;
    }
    for (int i = 0; i < 16; i++) {
        chip8->stack[i] = 0;
    }
    for (int i = 0; i < 4096; i++) {
        chip8->ram[i] = 0;
    }
    for(int i = 0; i < 16; i++) {
        chip8->keypad[i] = 0;
        chip8->prev_keypad[i] = 0;
    }
}

void LoadROM(CHIP8 *chip8, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open ROM file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > 4096 - CHIP8_ROM_ADDR) {
        fprintf(stderr, "ROM too large to fit in memory\n");
        fclose(file);
        return;
    }

    fread(chip8->ram + CHIP8_ROM_ADDR, 1, file_size, file);
    fclose(file);
}
