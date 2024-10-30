#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        error("Usage: %s <ROM file>", argv[0]);
        return EXIT_FAILURE;
    }

    CPU cpu;
    Memory memory;

    initCPU(&cpu);
    initMemory(&memory);

    if (loadROM(&memory, argv[1]) != 0) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 10; i++) {  // 10 cycles
        if (!cpu.halted) {
            executeNextInstruction(&cpu, memory.data);
        }
    }

    return EXIT_SUCCESS;
}
