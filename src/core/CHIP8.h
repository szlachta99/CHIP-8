#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

// overridable defaults
#ifndef CHIP8_ROM_ADDR
#define CHIP8_ROM_ADDR 0x200
#endif

#ifndef CHIP8_RAM_SIZE
#define CHIP8_RAM_SIZE 4096
#endif

typedef uint8_t RAM[CHIP8_RAM_SIZE  ]; // 4KB of RAM

typedef uint8_t Registers[16]; // 16 registers (V0 to VF)

typedef uint8_t Keypad[16]; // 16 keys (0x0 to 0xF)

typedef uint64_t Screen[32]; // 64x32 monochromatic screen

typedef uint16_t Stack[16]; // 16 levels of stack

typedef struct _CHIP8 {
    Registers registers;
    Screen screen;
    Stack stack;
    uint16_t index : 12;
    uint16_t program_counter : 12;
    uint16_t stack_pointer : 4;
    uint8_t delay_timer;
    uint8_t sound_timer;
    RAM ram;
    Keypad keypad;
    Keypad prev_keypad;
    // external flags
    uint8_t screen_changed : 1;
    
} CHIP8;

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

Instruction FetchInstruction(CHIP8 *chip8);
void ExecuteInstruction(CHIP8 *chip8, Instruction instruction);
void InitializeCHIP8(CHIP8 *chip8);
void LoadROM(CHIP8 *chip8, const char *filename);


#endif