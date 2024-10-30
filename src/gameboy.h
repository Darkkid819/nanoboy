#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "cpu.h"
#include "memory.h"

typedef struct {
    CPU cpu;
    Memory memory;
    int running;
} GameBoy;

void initGameBoy(GameBoy *gameBoy);
int loadGameBoyROM(GameBoy *gameBoy, const char *filePath);
void runGameBoy(GameBoy *gameBoy);
void stepGameBoy(GameBoy *gameBoy, int cycles);

#endif 
