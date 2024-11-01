#include "cpu.h"
#include "utils.h"



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

static void NOP(CPU *cpu, Memory *memory, Register reg1, Register reg2) {
    (void)cpu; (void)memory; (void)reg1; (void)reg2;
    p_instr("NOP executed");
}

// 8bit load/store/move instructions
static void LD_r_r(CPU *cpu, Memory *memory, Register dest, Register src) {
    (void)memory;
    uint8_t *rDest = getRegister(cpu, dest);
    uint8_t *rSrc = getRegister(cpu, src);
    if (rDest && rSrc) {
        *rDest = *rSrc;
        p_instr("LD %s <- %s: 0x%02X", getRegisterName(dest), getRegisterName(src), *rSrc);
    }
}

static void LD_r_d8(CPU *cpu, Memory *memory, Register dest, Register src) {
    (void)src;
    uint8_t *rDest = getRegister(cpu, dest);
    if (rDest) {
        *rDest = readByte(memory, cpu->pc++);
        p_instr("LD %s <- d8: 0x%02X", getRegisterName(dest), *rDest);
    }
}

static void LD_A_m(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint16_t address = (*getRegister(cpu, highReg) << 8) | *getRegister(cpu, lowReg);
    cpu->a = readByte(memory, address);
    p_instr("LD A <- (0x%04X): 0x%02X", address, cpu->a);
}

static void LD_m_A(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint16_t address = (*getRegister(cpu, highReg) << 8) | *getRegister(cpu, lowReg);
    writeByte(memory, address, cpu->a);
    p_instr("LD (0x%04X) <- A: 0x%02X", address, cpu->a);
}

static void LD_m_d8(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint16_t address = (*getRegister(cpu, highReg) << 8) | *getRegister(cpu, lowReg);
    uint8_t value = readByte(memory, cpu->pc++);
    writeByte(memory, address, value);
    p_instr("LD (0x%04X) <- d8: 0x%02X", address, value);
}

static void LDH_A_m(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = 0xFF00 | readByte(memory, cpu->pc++);
    cpu->a = readByte(memory, address);
    p_instr("LD A <- (0x%04X): 0x%02X", address, cpu->a);
}

static void LDH_m_A(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = 0xFF00 | readByte(memory, cpu->pc++);
    writeByte(memory, address, cpu->a);
    p_instr("LD (0x%04X) <- A: 0x%02X", address, cpu->a);
}

static void LD_A_a16(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = readWord(memory, cpu->pc);
    cpu->pc += 2;
    cpu->a = readByte(memory, address);
    p_instr("LD A <- (0x%04X): 0x%02X", address, cpu->a);
}

static void LD_a16_A(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = readWord(memory, cpu->pc);
    cpu->pc += 2;
    writeByte(memory, address, cpu->a);
    p_instr("LD (0x%04X) <- A: 0x%02X", address, cpu->a);
}

/*static void LD_rr_d16(CPU *cpu, uint8_t *memory, Register high, Register low) {*/
/*    uint8_t *rHigh = getRegister(cpu, high);*/
/*    uint8_t *rLow = getRegister(cpu, low);*/
/*    if (rHigh && rLow) {*/
/*        *rLow = memory[cpu->pc++];*/
/*        *rHigh = memory[cpu->pc++];*/
/*        p_instr("LD %s%s <- 0x%02X%02X", getRegisterName(high), getRegisterName(low), *rHigh, *rLow);*/
/*    }*/
/*}*/

/*static void INC(CPU *cpu, uint8_t *memory, Register reg, Register unused) {*/
/*    (void)memory; (void)unused;  */
/*    uint8_t *r = getRegister(cpu, reg);*/
/*    if (r) {*/
/*        (*r)++;*/
/*        cpu->f = (*r == 0) ? (cpu->f | 0x80) : (cpu->f & ~0x80);  // Set Zero flag if result is 0*/
/*        p_instr("INC %s -> 0x%02X", getRegisterName(reg), *r);*/
/*    }*/
/*}*/

// Opcode table with metadata
// format: instr | REG_1 | REG2 | cycles 
Instruction opcodeTable[256] = {
    [0x00] = { NOP, REG_NONE, REG_NONE, 4},         // NOP

    // 8bit load/store/move instructions
    [0x02] = { LD_m_A, REG_B, REG_C, 8 },           // LD (BC), A
    [0x06] = { LD_r_d8, REG_B, REG_NONE, 8 },       // LD B, d8
    [0x0A] = { LD_A_m, REG_B, REG_C, 8 },           // LD A, (BC)
    [0x0E] = { LD_r_d8, REG_C, REG_NONE, 8 },       // LD C, d8
    [0x12] = { LD_m_A, REG_D, REG_E, 8 },           // LD (DE), A
    [0x16] = { LD_r_d8, REG_D, REG_NONE, 8 },       // LD D, d8
    [0x1A] = { LD_A_m, REG_D, REG_E, 8 },           // LD A, (DE)
    [0x1E] = { LD_r_d8, REG_E, REG_NONE, 8 },       // LD E, d8
    [0x22] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL+), A
    [0x26] = { LD_r_d8, REG_H, REG_NONE, 8 },       // LD H, d8
    [0x2A] = { LD_A_m, REG_H, REG_L, 8 },           // LD A, (HL+)
    [0x2E] = { LD_r_d8, REG_L, REG_NONE, 8 },       // LD L, d8
    [0x32] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL-), A
    [0x36] = { LD_m_d8, REG_H, REG_L, 12 },         // LD (HL), d8
    [0x3A] = { LD_A_m, REG_H, REG_L, 8 },           // LD A, (HL-)
    [0x3E] = { LD_r_d8, REG_A, REG_NONE, 8 },       // LD A, d8
    [0x40] = { LD_r_r, REG_B, REG_B, 4 },           // LD B, B
    [0x41] = { LD_r_r, REG_B, REG_C, 4 },           // LD B, C
    [0x42] = { LD_r_r, REG_B, REG_D, 4 },           // LD B, D
    [0x43] = { LD_r_r, REG_B, REG_E, 4 },           // LD B, E
    [0x44] = { LD_r_r, REG_B, REG_H, 4 },           // LD B, H
    [0x45] = { LD_r_r, REG_B, REG_L, 4 },           // LD B, L
    [0x46] = { LD_A_m, REG_H, REG_L, 8 },           // LD B, (HL)
    [0x47] = { LD_r_r, REG_B, REG_A, 4 },           // LD B, A
    [0x48] = { LD_r_r, REG_C, REG_B, 4 },           // LD C, B
    [0x49] = { LD_r_r, REG_C, REG_C, 4 },           // LD C, C
    [0x4A] = { LD_r_r, REG_C, REG_D, 4 },           // LD C, D
    [0x4B] = { LD_r_r, REG_C, REG_E, 4 },           // LD C, E
    [0x4C] = { LD_r_r, REG_C, REG_H, 4 },           // LD C, H
    [0x4D] = { LD_r_r, REG_C, REG_L, 4 },           // LD C, L
    [0x4E] = { LD_A_m, REG_H, REG_L, 8 },           // LD C, (HL)
    [0x4F] = { LD_r_r, REG_C, REG_A, 4 },           // LD C, A
    [0x50] = { LD_r_r, REG_D, REG_B, 4 },           // LD D, B
    [0x51] = { LD_r_r, REG_D, REG_C, 4 },           // LD D, C
    [0x52] = { LD_r_r, REG_D, REG_D, 4 },           // LD D, D
    [0x53] = { LD_r_r, REG_D, REG_E, 4 },           // LD D, E
    [0x54] = { LD_r_r, REG_D, REG_H, 4 },           // LD D, H
    [0x55] = { LD_r_r, REG_D, REG_L, 4 },           // LD D, L
    [0x56] = { LD_A_m, REG_H, REG_L, 8 },           // LD D, (HL)
    [0x57] = { LD_r_r, REG_D, REG_A, 4 },           // LD D, A
    [0x58] = { LD_r_r, REG_E, REG_B, 4 },           // LD E, B
    [0x59] = { LD_r_r, REG_E, REG_C, 4 },           // LD E, C
    [0x5A] = { LD_r_r, REG_E, REG_D, 4 },           // LD E, D
    [0x5B] = { LD_r_r, REG_E, REG_E, 4 },           // LD E, E
    [0x5C] = { LD_r_r, REG_E, REG_H, 4 },           // LD E, H
    [0x5D] = { LD_r_r, REG_E, REG_L, 4 },           // LD E, L
    [0x5E] = { LD_A_m, REG_H, REG_L, 8 },           // LD E, (HL)
    [0x5F] = { LD_r_r, REG_E, REG_A, 4 },           // LD E, A
    [0x60] = { LD_r_r, REG_H, REG_B, 4 },           // LD H, B
    [0x61] = { LD_r_r, REG_H, REG_C, 4 },           // LD H, C
    [0x62] = { LD_r_r, REG_H, REG_D, 4 },           // LD H, D
    [0x63] = { LD_r_r, REG_H, REG_E, 4 },           // LD H, E
    [0x64] = { LD_r_r, REG_H, REG_H, 4 },           // LD H, H
    [0x65] = { LD_r_r, REG_H, REG_L, 4 },           // LD H, L
    [0x66] = { LD_A_m, REG_H, REG_L, 8 },           // LD H, (HL)
    [0x67] = { LD_r_r, REG_H, REG_A, 4 },           // LD H, A
    [0x68] = { LD_r_r, REG_L, REG_B, 4 },           // LD L, B
    [0x69] = { LD_r_r, REG_L, REG_C, 4 },           // LD L, C
    [0x6A] = { LD_r_r, REG_L, REG_D, 4 },           // LD L, D
    [0x6B] = { LD_r_r, REG_L, REG_E, 4 },           // LD L, E
    [0x6C] = { LD_r_r, REG_L, REG_H, 4 },           // LD L, H
    [0x6D] = { LD_r_r, REG_L, REG_L, 4 },           // LD L, L
    [0x6E] = { LD_A_m, REG_H, REG_L, 8 },           // LD L, (HL)
    [0x6F] = { LD_r_r, REG_L, REG_A, 4 },           // LD L, A
    [0x70] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), B
    [0x71] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), C
    [0x72] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), D
    [0x73] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), E
    [0x74] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), H
    [0x75] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), L
    [0x77] = { LD_m_A, REG_H, REG_L, 8 },           // LD (HL), A
    [0x78] = { LD_r_r, REG_A, REG_B, 4 },           // LD A, B
    [0x79] = { LD_r_r, REG_A, REG_C, 4 },           // LD A, C
    [0x7A] = { LD_r_r, REG_A, REG_D, 4 },           // LD A, D
    [0x7B] = { LD_r_r, REG_A, REG_E, 4 },           // LD A, E
    [0x7C] = { LD_r_r, REG_A, REG_H, 4 },           // LD A, H
    [0x7D] = { LD_r_r, REG_A, REG_L, 4 },           // LD A, L
    [0x7E] = { LD_A_m, REG_H, REG_L, 8 },           // LD A, (HL)
    [0x7F] = { LD_r_r, REG_A, REG_A, 4 },           // LD A, A
    [0xE0] = { LDH_m_A, REG_NONE, REG_NONE, 12 },   // LDH (a8), A
    [0xE2] = { LDH_m_A, REG_C, REG_NONE, 8 },       // LD (C), A
    [0xEA] = { LD_a16_A, REG_NONE, REG_NONE, 16 },  // LD (a16), A
    [0xF0] = { LDH_A_m, REG_NONE, REG_NONE, 12 },   // LDH A, (a8)
    [0xF2] = { LDH_A_m, REG_C, REG_NONE, 8 },       // LD A, (C)
    [0xFA] = { LD_A_a16, REG_NONE, REG_NONE, 16 },  // LD A, (a16)
};

void initCPU(CPU *cpu) {
    cpu->a = cpu->f = 0;
    cpu->b = cpu->c = cpu->d = cpu->e = 0;
    cpu->h = cpu->l = 0;
    cpu->sp = 0xFFFE;  // Stack pointer starts at the top of RAM
    cpu->pc = 0x0100;  // Program counter starts after BIOS
    cpu->ime = 1;      // Enable interrupts by default
    cpu->halted = 0;
    initTimer(&cpu->timer);

    debug("CPU Initialized");
}

void executeNextInstruction(CPU *cpu, Memory *memory) {
    uint8_t opcode = readByte(memory, cpu->pc++);
    Instruction instr = opcodeTable[opcode];

    if (instr.execute) {
        instr.execute(cpu, memory, instr.reg1, instr.reg2);
        addCycles(&cpu->timer, instr.cycles);
    } else {
        error("Unknown opcode: 0x%02X", opcode);
    }
}
