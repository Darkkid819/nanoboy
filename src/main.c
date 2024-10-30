#include <stdio.h>
#include "cpu.h"
#include "memory.h"
#include "config.h"
#include "utils.h"

int main() {
    error("hello from error");
    CPU cpu;
    Memory memory;

    initCPU(&cpu);
    initMemory(&memory);

    memory.data[0x0100] = 0x00;  // NOP opcode

    printf("Starting Emulation...\n");

    for (int i = 0; i < 10; i++) {  // 10 cycles
        if (!cpu.halted) {
            executeNextInstruction(&cpu, memory.data);
        }
    }

    printf("Emulation Complete\n");
    return 0;
}
