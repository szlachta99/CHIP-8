# Compiler and flags
CC = gcc
CDEBUGFLAGS = -fdiagnostics-color=always -g
CFLAGS = -Wall -std=c2x
LDFLAGS = -lm -lncurses

# Directories
SRC_DIR = src/core
ASM_DIR = src/assembler
DIS_DIR = src/disassembler
BUILD_DIR = build
EXECUTABLE = CHIP8
ASM_EXECUTABLE = ch8asm
DIS_EXECUTABLE = ch8dis

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

ASM_SRC = $(wildcard $(ASM_DIR)/*.c)
ASM_OBJ = $(patsubst $(ASM_DIR)/%.c,$(BUILD_DIR)/assembler/%.o,$(ASM_SRC))

DIS_SRC = $(wildcard $(DIS_DIR)/*.c)
DIS_OBJ = $(patsubst $(DIS_DIR)/%.c,$(BUILD_DIR)/disassembler/%.o,$(DIS_SRC))

# Default target
all: $(EXECUTABLE)

# Link main executable
$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

# Build assembler target
assembler: $(ASM_OBJ)
	$(CC) $(ASM_OBJ) -o $(ASM_EXECUTABLE)

# Build disassembler target
disassembler: $(DIS_OBJ)
	$(CC) $(DIS_OBJ) -o $(DIS_EXECUTABLE)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

$(BUILD_DIR)/assembler/%.o: $(ASM_DIR)/%.c | $(BUILD_DIR)/assembler
	$(CC) $(CFLAGS) -I$(ASM_DIR) -c $< -o $@

$(BUILD_DIR)/disassembler/%.o: $(DIS_DIR)/%.c | $(BUILD_DIR)/disassembler
	$(CC) $(CFLAGS) -I$(DIS_DIR) -c $< -o $@

# Create build subdirs
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/assembler:
	mkdir -p $(BUILD_DIR)/assembler

$(BUILD_DIR)/disassembler:
	mkdir -p $(BUILD_DIR)/disassembler

debug: CFLAGS += $(CDEBUGFLAGS)
debug: all

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE) $(ASM_EXECUTABLE) $(DIS_EXECUTABLE)

.PHONY: all clean run debug assembler disassembler
