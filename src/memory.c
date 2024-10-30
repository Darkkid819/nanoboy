#include "memory.h"
#include <stdio.h>
#include <string.h> 

void initMemory(Memory *memory) {
    memset(memory->data, 0, MEMORY_SIZE);
    printf("Memory Initialized\n");
}

uint8_t readByte(Memory *memory, uint16_t address) {
    return memory->data[address];
}

void writeByte(Memory *memory, uint16_t address, uint8_t value) {
    memory->data[address] = value;
}
