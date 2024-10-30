#include "memory.h"
#include "utils.h"
#include <stdio.h>
#include <string.h> 

void initMemory(Memory *memory) {
    memset(memory->data, 0, MEMORY_SIZE);
    debug("Memory Initialized");
}

uint8_t readByte(Memory *memory, uint16_t address) {
    return memory->data[address];
}

void writeByte(Memory *memory, uint16_t address, uint8_t value) {
    memory->data[address] = value;
}

int loadROM(Memory *memory, const char *filePath) {
    FILE *file = fopen(filePath, "rb");
    if (!file) {
        error("Failed to open ROM: %s", filePath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize > (MEMORY_SIZE - 0x0100)) {
        error("ROM size exceeds available memory");
        fclose(file);
        return -1;
    }

    fread(&memory->data[0x0100], 1, fileSize, file);
    fclose(file);

    success("ROM loaded: %s (%ld bytes)", filePath, fileSize);
    return 0;
}
