#include "cpu.h"
#include <stdio.h>

void initCPU(CPU *cpu) {
    cpu->a = cpu->f = 0;
    cpu->b = cpu->c = cpu->d = cpu->e = 0;
    cpu->h = cpu->l = 0;
    cpu->sp = 0xFFFE;  // Stack pointer starts at the top of RAM
    cpu->pc = 0x0100;  // Program counter starts after BIOS
    cpu->ime = 1;  // Enable interrupts by default
    cpu->halted = 0;

    printf("CPU Initialized\n");
}

void executeNextInstruction(CPU *cpu, uint8_t *memory) {
    uint8_t opcode = memory[cpu->pc++];
    
    if (opcode == 0x00) {
        printf("NOP executed\n");
        return;
    } else {
        printf("Unknown opcode: 0x%02X\n", opcode);
    }
}
