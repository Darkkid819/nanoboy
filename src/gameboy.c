#include "gameboy.h"
#include "utils.h"
#include <stdio.h>
#include <stdbool.h>

void initGameBoy(GameBoy *gameBoy) {
    initCPU(&gameBoy->cpu);
    initMemory(&gameBoy->memory);
    gameBoy->running = true;
    debug("Game Boy Initialized");
}

int loadGameBoyROM(GameBoy *gameBoy, const char *filePath) {
    return loadROM(&gameBoy->memory, filePath);
}

void runGameBoy(GameBoy *gameBoy) {
    while (gameBoy->running) {
        if (!gameBoy->cpu.halted) {
            executeNextInstruction(&gameBoy->cpu, gameBoy->memory.data);
        } else {
            debug("CPU halted, stopping execution");
            gameBoy->running = false;
        }
    }
}

void stepGameBoy(GameBoy *gameBoy, int cycles) {
    for (int i = 0; i < cycles; i++) {
        if (!gameBoy->cpu.halted) {
            executeNextInstruction(&gameBoy->cpu, gameBoy->memory.data);
        } else {
            debug("CPU halted, stopping execution");
            break;
        }
    }
}
