#include "cpu.h"
#include "utils.h"

static void NOP(CPU *cpu, uint8_t *memory) {
    p_instr("NOP executed");
}

InstructionFP opcodeTable[256] = {
    [0x00] = NOP,
};

void initCPU(CPU *cpu) {
    cpu->a = cpu->f = 0;
    cpu->b = cpu->c = cpu->d = cpu->e = 0;
    cpu->h = cpu->l = 0;
    cpu->sp = 0xFFFE;  // Stack pointer starts at the top of RAM
    cpu->pc = 0x0100;  // Program counter starts after BIOS
    cpu->ime = 1;  // Enable interrupts by default
    cpu->halted = 0;

    debug("CPU Initialized");
}

void executeNextInstruction(CPU *cpu, uint8_t *memory) {
    uint8_t opcode = memory[cpu->pc++];
    InstructionFP instr = opcodeTable[opcode];
    
    if (instr) {
        instr(cpu, memory);
    } else {
        error("Unknown opcode: 0x%02X", opcode);
    }
}
