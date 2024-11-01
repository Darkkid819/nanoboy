#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t data[MEMORY_SIZE];  // 64KB of addressable memory
} Memory;

void initMemory(Memory *memory);
uint8_t readByte(Memory *memory, uint16_t address);
uint16_t readWord(Memory *memory, uint16_t address);
void writeByte(Memory *memory, uint16_t address, uint8_t value);
int loadROM(Memory *memory, const char *filePath);  

#endif 
