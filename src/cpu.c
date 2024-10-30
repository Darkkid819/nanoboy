#include "cpu.h"
#include "utils.h"

static uint16_t read16(uint8_t *memory, uint16_t address) {
    return memory[address] | (memory[address + 1] << 8);
}

static uint8_t* getRegister(CPU *cpu, Register reg) {
    switch (reg) {
        case REG_B: return &cpu->b;
        case REG_C: return &cpu->c;
        case REG_D: return &cpu->d;
        case REG_E: return &cpu->e;
        case REG_H: return &cpu->h;
        case REG_L: return &cpu->l;
        case REG_A: return &cpu->a;
        default: return NULL;
    }
}

static const char* getRegisterName(Register reg) {
    switch (reg) {
        case REG_B: return "B";
        case REG_C: return "C";
        case REG_D: return "D";
        case REG_E: return "E";
        case REG_H: return "H";
        case REG_L: return "L";
        case REG_A: return "A";
        case REG_NONE: return "NONE";
        default: return "UNKNOWN";
    }
}

static void NOP(CPU *cpu, uint8_t *memory, Register reg1, Register reg2) {
    (void)cpu; (void)memory; (void)reg1; (void)reg2;
    p_instr("NOP executed");
}

static void LD(CPU *cpu, uint8_t *memory, Register dest, Register src) {
    (void)memory; 
    uint8_t *rDest = getRegister(cpu, dest);
    uint8_t *rSrc = getRegister(cpu, src);
    if (rDest && rSrc) {
        *rDest = *rSrc;
        p_instr("LD %s <- %s: 0x%02X", getRegisterName(dest), getRegisterName(src), *rSrc);
    }
}

static void LD_rr_d16(CPU *cpu, uint8_t *memory, Register high, Register low) {
    uint8_t *rHigh = getRegister(cpu, high);
    uint8_t *rLow = getRegister(cpu, low);
    if (rHigh && rLow) {
        *rLow = memory[cpu->pc++];
        *rHigh = memory[cpu->pc++];
        p_instr("LD %s%s <- 0x%02X%02X", getRegisterName(high), getRegisterName(low), *rHigh, *rLow);
    }
}

static void INC(CPU *cpu, uint8_t *memory, Register reg, Register unused) {
    (void)memory; (void)unused;  
    uint8_t *r = getRegister(cpu, reg);
    if (r) {
        (*r)++;
        cpu->f = (*r == 0) ? (cpu->f | 0x80) : (cpu->f & ~0x80);  // Set Zero flag if result is 0
        p_instr("INC %s -> 0x%02X", getRegisterName(reg), *r);
    }
}

// Opcode table with metadata
Instruction opcodeTable[256] = {
    [0x00] = { NOP, REG_NONE, REG_NONE },               // NOP
    [0x01] = { LD_rr_d16, REG_B, REG_C },               // LD BC, d16
    [0x04] = { INC, REG_B, REG_NONE },                  // INC B
    [0x78] = { LD, REG_A, REG_B },                      // LD A, B
};

void initCPU(CPU *cpu) {
    cpu->a = cpu->f = 0;
    cpu->b = cpu->c = cpu->d = cpu->e = 0;
    cpu->h = cpu->l = 0;
    cpu->sp = 0xFFFE;  // Stack pointer starts at the top of RAM
    cpu->pc = 0x0100;  // Program counter starts after BIOS
    cpu->ime = 1;      // Enable interrupts by default
    cpu->halted = 0;

    debug("CPU Initialized");
}

void executeNextInstruction(CPU *cpu, uint8_t *memory) {
    uint8_t opcode = memory[cpu->pc++];
    Instruction instr = opcodeTable[opcode];

    if (instr.execute) {
        instr.execute(cpu, memory, instr.reg1, instr.reg2);
    } else {
        error("Unknown opcode: 0x%02X", opcode);
    }
}
