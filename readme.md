# CHIP-8 Emulator

A simple CHIP-8 emulator implementation written in C. This project aims to provide a basic but functional emulator for running CHIP-8 games and applications.

## What is CHIP-8?

CHIP-8 is an interpreted programming language developed in the 1970s, designed to allow video games to be more easily programmed for early microcomputers. It was not a physical computing system, but rather a virtual machine that ran on actual hardware.

## Features

- Full CHIP-8 instruction set implementation
- Simple display rendering
- Keyboard input handling
- Sound support
- Configurable clock speed

## Getting Started

### Prerequisites

- linux
- SDL3
- make

### Build

```bash
make # builds the emulator
make assembler # builds assembler
make disassembler # builds disassembler
```

### Usage

```bash
CHIP8 < ROM file >
```

You can find roms [here](https://github.com/kripod/chip8-roms)

## Controls

The emulator uses standard key mapping.
The keypad is mapped to qwerty keyboard as follows:

``` none
1 2 3 4 | 1 2 3 C
q w e r | 4 5 6 D
a s d f | 7 8 9 E
z x c v | A 0 B F
```

## Implementation Details

The emulator implements the following components:

- CPU with 35 opcodes
- 4K memory
- 16 8-bit registers
- 64×32 pixel monochrome display
- 16-key hexadecimal keypad
- Two timers (delay and sound)

## Resources

- [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [CHIP-8 Variant Opcode Table](https://chip8.gulrak.net)

## Acknowledgments

- ChatGPT
- Github Copilot
