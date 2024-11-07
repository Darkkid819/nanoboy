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
        case REG_F: return &cpu->f;
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
        case REG_F: return "F";
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

// 16 bit load/store/move instructions
static void LD_SP_d16(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    cpu->sp = readWord(memory, cpu->pc);
    cpu->pc += 2;
    p_instr("LD SP <- d16: 0x%04X", cpu->sp);
}

static void LD_rr_d16(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint8_t *high = getRegister(cpu, highReg);
    uint8_t *low = getRegister(cpu, lowReg);
    if (high && low) {
        *low = readByte(memory, cpu->pc++);
        *high = readByte(memory, cpu->pc++);
        p_instr("LD %s%s <- d16: 0x%02X%02X", getRegisterName(highReg), getRegisterName(lowReg), *high, *low);
    }
}

static void LD_a16_SP(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = readWord(memory, cpu->pc);
    cpu->pc += 2;
    writeByte(memory, address, cpu->sp & 0xFF);        
    writeByte(memory, address + 1, (cpu->sp >> 8) & 0xFF);  
    p_instr("LD (0x%04X) <- SP: 0x%04X", address, cpu->sp);
}


static void POP_rr(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint8_t *high = getRegister(cpu, highReg);
    uint8_t *low = getRegister(cpu, lowReg);
    if (high && low) {
        *low = readByte(memory, cpu->sp++);
        *high = readByte(memory, cpu->sp++);
        p_instr("POP %s%s: 0x%02X%02X", getRegisterName(highReg), getRegisterName(lowReg), *high, *low);
    }
}

static void PUSH_rr(CPU *cpu, Memory *memory, Register highReg, Register lowReg) {
    uint8_t *high = getRegister(cpu, highReg);
    uint8_t *low = getRegister(cpu, lowReg);
    if (high && low) {
        writeByte(memory, --cpu->sp, *high);
        writeByte(memory, --cpu->sp, *low);
        p_instr("PUSH %s%s: 0x%02X%02X", getRegisterName(highReg), getRegisterName(lowReg), *high, *low);
    }
}

static void LD_SP_HL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)memory; (void)unused1; (void)unused2;
    cpu->sp = (cpu->h << 8) | cpu->l;
    p_instr("LD SP <- HL: 0x%04X", cpu->sp);
}


static void LD_HL_SP_plus_s8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    int8_t offset = (int8_t)readByte(memory, cpu->pc++);
    uint16_t result = cpu->sp + offset;
    cpu->h = (result >> 8) & 0xFF;
    cpu->l = result & 0xFF;
    p_instr("LD HL <- SP + s8: SP=0x%04X, s8=%d, Result=0x%04X", cpu->sp, offset, result);
}

// 8bit arithmetic/logical instructions
static void setFlagsInc(CPU *cpu, uint8_t result) {
    // Z flag
    cpu->f = (result == 0) ? (cpu->f | 0x80) : (cpu->f & ~0x80);
    // N flag is cleared
    cpu->f &= ~0x40;
    // H flag is set if there was a carry from bit 3
    cpu->f = (result & 0x0F) == 0 ? (cpu->f | 0x20) : (cpu->f & ~0x20);
    // C flag is unaffected
}

static void setFlagsDec(CPU *cpu, uint8_t result) {
    // Z flag
    cpu->f = (result == 0) ? (cpu->f | 0x80) : (cpu->f & ~0x80);
    // N flag is set
    cpu->f |= 0x40;
    // H flag is set if there was a borrow from bit 4
    cpu->f = (result & 0x0F) == 0x0F ? (cpu->f | 0x20) : (cpu->f & ~0x20);
    // C flag is unaffected
}

static void INC_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *r = getRegister(cpu, reg);
    if (r) {
        (*r)++;
        setFlagsInc(cpu, *r);
        p_instr("INC %s -> 0x%02X", getRegisterName(reg), *r);
    }
}

static void DEC_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *r = getRegister(cpu, reg);
    if (r) {
        (*r)--;
        setFlagsDec(cpu, *r);
        p_instr("DEC %s -> 0x%02X", getRegisterName(reg), *r);
    }
}

static void INC_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = readByte(memory, address) + 1;
    writeByte(memory, address, value);
    setFlagsInc(cpu, value);
    p_instr("INC (HL) -> 0x%02X at 0x%04X", value, address);
}

static void DEC_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = readByte(memory, address) - 1;
    writeByte(memory, address, value);
    setFlagsDec(cpu, value);
    p_instr("DEC (HL) -> 0x%02X at 0x%04X", value, address);
}

static void DAA(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)memory; (void)unused1; (void)unused2;

    uint8_t adjust = 0;
    uint8_t carryFlag = cpu->f & 0x10; // CY flag
    uint8_t halfCarryFlag = cpu->f & 0x20; // H flag

    if (halfCarryFlag || ((cpu->a & 0x0F) > 9)) {
        adjust |= 0x06;
    }
    if (carryFlag || (cpu->a > 0x99)) {
        adjust |= 0x60;
        cpu->f |= 0x10; // Set CY
    } else {
        cpu->f &= ~0x10; // Clear CY
    }

    cpu->a = cpu->f & 0x40 ? cpu->a - adjust : cpu->a + adjust; // Adjust A based on N flag
    cpu->f = (cpu->a == 0 ? cpu->f | 0x80 : cpu->f & ~0x80); // Set or clear Z flag
    cpu->f &= ~0x20; // Clear H flag
    p_instr("DAA: A=0x%02X, Adjust=0x%02X", cpu->a, adjust);
}

static void SCF(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)memory; (void)unused1; (void)unused2;

    cpu->f = (cpu->f & ~0x60) | 0x10; // Set CY, clear N and H
    p_instr("SCF: CY set, N and H cleared");
}

static void CPL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)memory; (void)unused1; (void)unused2;

    cpu->a = ~cpu->a;
    cpu->f |= 0x60; // Set N and H flags
    p_instr("CPL: A=0x%02X (complemented)", cpu->a);
}

static void CCF(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)memory; (void)unused1; (void)unused2;

    uint8_t carryFlag = cpu->f & 0x10;
    cpu->f = (cpu->f & ~0x60) | (!carryFlag << 4); // Toggle CY, clear N and H
    p_instr("CCF: CY=%d (toggled)", !carryFlag);
}

static void setFlagsAdd(CPU *cpu, uint8_t result, uint8_t operand, uint8_t carry) {
    cpu->f = (result == 0 ? 0x80 : 0);  // Set Z flag if result is zero
    cpu->f |= (result < operand + carry) ? 0x10 : 0; // Set C flag if overflow
    cpu->f |= (((cpu->a & 0xF) + (operand & 0xF) + carry) & 0x10) ? 0x20 : 0; // Set H flag if half-carry
}

static void ADD_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        uint8_t operand = *regValue;
        uint8_t result = cpu->a + operand;
        setFlagsAdd(cpu, result, operand, 0);
        cpu->a = result;
        p_instr("ADD A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void ADD_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t operand = readByte(memory, address);
    uint8_t result = cpu->a + operand;
    setFlagsAdd(cpu, result, operand, 0);
    cpu->a = result;
    p_instr("ADD A, (HL): 0x%02X", cpu->a);
}

static void ADD_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint8_t operand = readByte(memory, cpu->pc++);
    uint8_t result = cpu->a + operand;
    setFlagsAdd(cpu, result, operand, 0);
    cpu->a = result;
    p_instr("ADD A, d8: 0x%02X", cpu->a);
}

static void ADC_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        uint8_t operand = *regValue;
        uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
        uint8_t result = cpu->a + operand + carry;
        setFlagsAdd(cpu, result, operand, carry);
        cpu->a = result;
        p_instr("ADC A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void ADC_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t operand = readByte(memory, address);
    uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
    uint8_t result = cpu->a + operand + carry;
    setFlagsAdd(cpu, result, operand, carry);
    cpu->a = result;
    p_instr("ADC A, (HL): 0x%02X", cpu->a);
}


static void ADC_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint8_t operand = readByte(memory, cpu->pc++);
    uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
    uint8_t result = cpu->a + operand + carry;
    setFlagsAdd(cpu, result, operand, carry);
    cpu->a = result;
    p_instr("ADC A, d8: 0x%02X", cpu->a);
}

static void setFlagsSub(CPU *cpu, uint8_t result, uint8_t operand, uint8_t carry) {
    cpu->f = (result == 0 ? 0x80 : 0);              // Set Z flag if result is zero
    cpu->f |= 0x40;                                 // Set N flag (subtraction)
    cpu->f |= (operand + carry > cpu->a) ? 0x10 : 0; // Set C flag if borrow occurred
    cpu->f |= (((cpu->a & 0xF) - (operand & 0xF) - carry) & 0x10) ? 0x20 : 0; // Set H flag if half-borrow
}

static void SUB_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        uint8_t operand = *regValue;
        uint8_t result = cpu->a - operand;
        setFlagsSub(cpu, result, operand, 0);
        cpu->a = result;
        p_instr("SUB A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void SUB_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t operand = readByte(memory, address);
    uint8_t result = cpu->a - operand;
    setFlagsSub(cpu, result, operand, 0);
    cpu->a = result;
    p_instr("SUB A, (HL): 0x%02X", cpu->a);
}

static void SUB_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint8_t operand = readByte(memory, cpu->pc++);
    uint8_t result = cpu->a - operand;
    setFlagsSub(cpu, result, operand, 0);
    cpu->a = result;
    p_instr("SUB A, d8: 0x%02X", cpu->a);
}

static void SBC_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        uint8_t operand = *regValue;
        uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
        uint8_t result = cpu->a - operand - carry;
        setFlagsSub(cpu, result, operand, carry);
        cpu->a = result;
        p_instr("SBC A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void SBC_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t operand = readByte(memory, address);
    uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
    uint8_t result = cpu->a - operand - carry;
    setFlagsSub(cpu, result, operand, carry);
    cpu->a = result;
    p_instr("SBC A, (HL): 0x%02X", cpu->a);
}

static void SBC_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint8_t operand = readByte(memory, cpu->pc++);
    uint8_t carry = (cpu->f & 0x10) ? 1 : 0;
    uint8_t result = cpu->a - operand - carry;
    setFlagsSub(cpu, result, operand, carry);
    cpu->a = result;
    p_instr("SBC A, d8: 0x%02X", cpu->a);
}

static void setFlagsAnd(CPU *cpu, uint8_t result) {
    cpu->f = (result == 0 ? 0x80 : 0x00);  // Set Z flag if result is zero
    cpu->f |= 0x20;                        // Set H flag
}

static void setFlagsXor(CPU *cpu, uint8_t result) {
    cpu->f = (result == 0 ? 0x80 : 0x00);  // Set Z flag if result is zero
}

static void AND_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        cpu->a &= *regValue;
        setFlagsAnd(cpu, cpu->a);
        p_instr("AND A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void AND_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    cpu->a &= readByte(memory, address);
    setFlagsAnd(cpu, cpu->a);
    p_instr("AND A, (HL): 0x%02X", cpu->a);
}

static void AND_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    cpu->a &= readByte(memory, cpu->pc++);
    setFlagsAnd(cpu, cpu->a);
    p_instr("AND A, d8: 0x%02X", cpu->a);
}

static void XOR_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        cpu->a ^= *regValue;
        setFlagsXor(cpu, cpu->a);
        p_instr("XOR A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void XOR_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    cpu->a ^= readByte(memory, address);
    setFlagsXor(cpu, cpu->a);
    p_instr("XOR A, (HL): 0x%02X", cpu->a);
}

static void XOR_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    cpu->a ^= readByte(memory, cpu->pc++);
    setFlagsXor(cpu, cpu->a);
    p_instr("XOR A, d8: 0x%02X", cpu->a);
}

static void setFlagsOr(CPU *cpu, uint8_t result) {
    cpu->f = (result == 0 ? 0x80 : 0x00);  // Set Z flag if result is zero
}

static void setFlagsCp(CPU *cpu, uint8_t a, uint8_t operand) {
    uint8_t result = a - operand;
    cpu->f = 0x40;  // Set N flag
    if (result == 0) cpu->f |= 0x80;  // Set Z flag
    if ((a & 0x0F) < (operand & 0x0F)) cpu->f |= 0x20;  // Set H flag
    if (a < operand) cpu->f |= 0x10;  // Set C flag
}

static void OR_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        cpu->a |= *regValue;
        setFlagsOr(cpu, cpu->a);
        p_instr("OR A, %s: 0x%02X", getRegisterName(reg), cpu->a);
    }
}

static void OR_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    cpu->a |= readByte(memory, address);
    setFlagsOr(cpu, cpu->a);
    p_instr("OR A, (HL): 0x%02X", cpu->a);
}

static void OR_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    cpu->a |= readByte(memory, cpu->pc++);
    setFlagsOr(cpu, cpu->a);
    p_instr("OR A, d8: 0x%02X", cpu->a);
}

static void CP_A_r(CPU *cpu, Memory *memory, Register reg, Register unused) {
    (void)memory; (void)unused;
    uint8_t *regValue = getRegister(cpu, reg);
    if (regValue) {
        setFlagsCp(cpu, cpu->a, *regValue);
        p_instr("CP A, %s: A=0x%02X, operand=0x%02X", getRegisterName(reg), cpu->a, *regValue);
    }
}

static void CP_A_mHL(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = readByte(memory, address);
    setFlagsCp(cpu, cpu->a, value);
    p_instr("CP A, (HL): A=0x%02X, operand=0x%02X", cpu->a, value);
}

static void CP_A_d8(CPU *cpu, Memory *memory, Register unused1, Register unused2) {
    (void)unused1; (void)unused2;
    uint8_t value = readByte(memory, cpu->pc++);
    setFlagsCp(cpu, cpu->a, value);
    p_instr("CP A, d8: A=0x%02X, operand=0x%02X", cpu->a, value);
}


// Opcode table with metadata
// format: instr | REG_1 | REG2 | cycles 
Instruction opcodeTable[256] = {
    [0x00] = { NOP, REG_NONE, REG_NONE, 4},                 // NOP

    // 8bit load/store/move instructions
    [0x02] = { LD_m_A, REG_B, REG_C, 8 },                   // LD (BC), A
    [0x06] = { LD_r_d8, REG_B, REG_NONE, 8 },               // LD B, d8
    [0x0A] = { LD_A_m, REG_B, REG_C, 8 },                   // LD A, (BC)
    [0x0E] = { LD_r_d8, REG_C, REG_NONE, 8 },               // LD C, d8
    [0x12] = { LD_m_A, REG_D, REG_E, 8 },                   // LD (DE), A
    [0x16] = { LD_r_d8, REG_D, REG_NONE, 8 },               // LD D, d8
    [0x1A] = { LD_A_m, REG_D, REG_E, 8 },                   // LD A, (DE)
    [0x1E] = { LD_r_d8, REG_E, REG_NONE, 8 },               // LD E, d8
    [0x22] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL+), A
    [0x26] = { LD_r_d8, REG_H, REG_NONE, 8 },               // LD H, d8
    [0x2A] = { LD_A_m, REG_H, REG_L, 8 },                   // LD A, (HL+)
    [0x2E] = { LD_r_d8, REG_L, REG_NONE, 8 },               // LD L, d8
    [0x32] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL-), A
    [0x36] = { LD_m_d8, REG_H, REG_L, 12 },                 // LD (HL), d8
    [0x3A] = { LD_A_m, REG_H, REG_L, 8 },                   // LD A, (HL-)
    [0x3E] = { LD_r_d8, REG_A, REG_NONE, 8 },               // LD A, d8
    [0x40] = { LD_r_r, REG_B, REG_B, 4 },                   // LD B, B
    [0x41] = { LD_r_r, REG_B, REG_C, 4 },                   // LD B, C
    [0x42] = { LD_r_r, REG_B, REG_D, 4 },                   // LD B, D
    [0x43] = { LD_r_r, REG_B, REG_E, 4 },                   // LD B, E
    [0x44] = { LD_r_r, REG_B, REG_H, 4 },                   // LD B, H
    [0x45] = { LD_r_r, REG_B, REG_L, 4 },                   // LD B, L
    [0x46] = { LD_A_m, REG_H, REG_L, 8 },                   // LD B, (HL)
    [0x47] = { LD_r_r, REG_B, REG_A, 4 },                   // LD B, A
    [0x48] = { LD_r_r, REG_C, REG_B, 4 },                   // LD C, B
    [0x49] = { LD_r_r, REG_C, REG_C, 4 },                   // LD C, C
    [0x4A] = { LD_r_r, REG_C, REG_D, 4 },                   // LD C, D
    [0x4B] = { LD_r_r, REG_C, REG_E, 4 },                   // LD C, E
    [0x4C] = { LD_r_r, REG_C, REG_H, 4 },                   // LD C, H
    [0x4D] = { LD_r_r, REG_C, REG_L, 4 },                   // LD C, L
    [0x4E] = { LD_A_m, REG_H, REG_L, 8 },                   // LD C, (HL)
    [0x4F] = { LD_r_r, REG_C, REG_A, 4 },                   // LD C, A
    [0x50] = { LD_r_r, REG_D, REG_B, 4 },                   // LD D, B
    [0x51] = { LD_r_r, REG_D, REG_C, 4 },                   // LD D, C
    [0x52] = { LD_r_r, REG_D, REG_D, 4 },                   // LD D, D
    [0x53] = { LD_r_r, REG_D, REG_E, 4 },                   // LD D, E
    [0x54] = { LD_r_r, REG_D, REG_H, 4 },                   // LD D, H
    [0x55] = { LD_r_r, REG_D, REG_L, 4 },                   // LD D, L
    [0x56] = { LD_A_m, REG_H, REG_L, 8 },                   // LD D, (HL)
    [0x57] = { LD_r_r, REG_D, REG_A, 4 },                   // LD D, A
    [0x58] = { LD_r_r, REG_E, REG_B, 4 },                   // LD E, B
    [0x59] = { LD_r_r, REG_E, REG_C, 4 },                   // LD E, C
    [0x5A] = { LD_r_r, REG_E, REG_D, 4 },                   // LD E, D
    [0x5B] = { LD_r_r, REG_E, REG_E, 4 },                   // LD E, E
    [0x5C] = { LD_r_r, REG_E, REG_H, 4 },                   // LD E, H
    [0x5D] = { LD_r_r, REG_E, REG_L, 4 },                   // LD E, L
    [0x5E] = { LD_A_m, REG_H, REG_L, 8 },                   // LD E, (HL)
    [0x5F] = { LD_r_r, REG_E, REG_A, 4 },                   // LD E, A
    [0x60] = { LD_r_r, REG_H, REG_B, 4 },                   // LD H, B
    [0x61] = { LD_r_r, REG_H, REG_C, 4 },                   // LD H, C
    [0x62] = { LD_r_r, REG_H, REG_D, 4 },                   // LD H, D
    [0x63] = { LD_r_r, REG_H, REG_E, 4 },                   // LD H, E
    [0x64] = { LD_r_r, REG_H, REG_H, 4 },                   // LD H, H
    [0x65] = { LD_r_r, REG_H, REG_L, 4 },                   // LD H, L
    [0x66] = { LD_A_m, REG_H, REG_L, 8 },                   // LD H, (HL)
    [0x67] = { LD_r_r, REG_H, REG_A, 4 },                   // LD H, A
    [0x68] = { LD_r_r, REG_L, REG_B, 4 },                   // LD L, B
    [0x69] = { LD_r_r, REG_L, REG_C, 4 },                   // LD L, C
    [0x6A] = { LD_r_r, REG_L, REG_D, 4 },                   // LD L, D
    [0x6B] = { LD_r_r, REG_L, REG_E, 4 },                   // LD L, E
    [0x6C] = { LD_r_r, REG_L, REG_H, 4 },                   // LD L, H
    [0x6D] = { LD_r_r, REG_L, REG_L, 4 },                   // LD L, L
    [0x6E] = { LD_A_m, REG_H, REG_L, 8 },                   // LD L, (HL)
    [0x6F] = { LD_r_r, REG_L, REG_A, 4 },                   // LD L, A
    [0x70] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), B
    [0x71] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), C
    [0x72] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), D
    [0x73] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), E
    [0x74] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), H
    [0x75] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), L
    [0x77] = { LD_m_A, REG_H, REG_L, 8 },                   // LD (HL), A
    [0x78] = { LD_r_r, REG_A, REG_B, 4 },                   // LD A, B
    [0x79] = { LD_r_r, REG_A, REG_C, 4 },                   // LD A, C
    [0x7A] = { LD_r_r, REG_A, REG_D, 4 },                   // LD A, D
    [0x7B] = { LD_r_r, REG_A, REG_E, 4 },                   // LD A, E
    [0x7C] = { LD_r_r, REG_A, REG_H, 4 },                   // LD A, H
    [0x7D] = { LD_r_r, REG_A, REG_L, 4 },                   // LD A, L
    [0x7E] = { LD_A_m, REG_H, REG_L, 8 },                   // LD A, (HL)
    [0x7F] = { LD_r_r, REG_A, REG_A, 4 },                   // LD A, A
    [0xE0] = { LDH_m_A, REG_NONE, REG_NONE, 12 },           // LDH (a8), A
    [0xE2] = { LDH_m_A, REG_C, REG_NONE, 8 },               // LD (C), A
    [0xEA] = { LD_a16_A, REG_NONE, REG_NONE, 16 },          // LD (a16), A
    [0xF0] = { LDH_A_m, REG_NONE, REG_NONE, 12 },           // LDH A, (a8)
    [0xF2] = { LDH_A_m, REG_C, REG_NONE, 8 },               // LD A, (C)
    [0xFA] = { LD_A_a16, REG_NONE, REG_NONE, 16 },          // LD A, (a16)
    
    // 16 bit load/store/move instructions
    [0x01] = { LD_rr_d16, REG_B, REG_C, 12 },               // LD BC, d16
    [0x08] = { LD_a16_SP, REG_NONE, REG_NONE, 20 },         // LD (a16), SP
    [0x11] = { LD_rr_d16, REG_D, REG_E, 12 },               // LD DE, d16
    [0x21] = { LD_rr_d16, REG_H, REG_L, 12 },               // LD HL, d16
    [0x31] = { LD_SP_d16, REG_NONE, REG_NONE, 12 },         // LD SP, d16
    [0xC1] = { POP_rr, REG_B, REG_C, 12 },                  // POP BC
    [0xC5] = { PUSH_rr, REG_B, REG_C, 16 },                 // PUSH BC
    [0xD1] = { POP_rr, REG_D, REG_E, 12 },                  // POP DE
    [0xD5] = { PUSH_rr, REG_D, REG_E, 16 },                 // PUSH DE
    [0xE1] = { POP_rr, REG_H, REG_L, 12 },                  // POP HL
    [0xE5] = { PUSH_rr, REG_H, REG_L, 16 },                 // PUSH HL
    [0xF1] = { POP_rr, REG_A, REG_F, 12 },                  // POP AF
    [0xF5] = { PUSH_rr, REG_A, REG_F, 16 },                 // PUSH AF
    [0xF8] = { LD_HL_SP_plus_s8, REG_NONE, REG_NONE, 12 },  // LD HL, SP+s8
    [0xF9] = { LD_SP_HL, REG_NONE, REG_NONE, 8 },           // LD SP, HL

    // 8bit arithmetic/logical instructions
    [0x04] = { INC_r, REG_B, REG_NONE, 4 },                 // INC B
    [0x05] = { DEC_r, REG_B, REG_NONE, 4 },                 // DEC B
    [0x0C] = { INC_r, REG_C, REG_NONE, 4 },                 // INC C
    [0x0D] = { DEC_r, REG_C, REG_NONE, 4 },                 // DEC C
    [0x14] = { INC_r, REG_D, REG_NONE, 4 },                 // INC D
    [0x15] = { DEC_r, REG_D, REG_NONE, 4 },                 // DEC D
    [0x1C] = { INC_r, REG_E, REG_NONE, 4 },                 // INC E
    [0x1D] = { DEC_r, REG_E, REG_NONE, 4 },                 // DEC E
    [0x24] = { INC_r, REG_H, REG_NONE, 4 },                 // INC H
    [0x25] = { DEC_r, REG_H, REG_NONE, 4 },                 // DEC H
    [0x27] = { DAA, REG_NONE, REG_NONE, 4 },                // DAA
    [0x2C] = { INC_r, REG_L, REG_NONE, 4 },                 // INC L
    [0x2D] = { DEC_r, REG_L, REG_NONE, 4 },                 // DEC L
    [0x2F] = { CPL, REG_NONE, REG_NONE, 4 },                // CPL
    [0x34] = { INC_mHL, REG_NONE, REG_NONE, 12 },           // INC (HL)
    [0x35] = { DEC_mHL, REG_NONE, REG_NONE, 12 },           // DEC (HL)
    [0x37] = { SCF, REG_NONE, REG_NONE, 4 },                // SCF
    [0x3C] = { INC_r, REG_A, REG_NONE, 4 },                 // INC A
    [0x3D] = { DEC_r, REG_A, REG_NONE, 4 },                 // DEC A
    [0x3F] = { CCF, REG_NONE, REG_NONE, 4 },                // CCF
    [0x80] = { ADD_A_r, REG_B, REG_NONE, 4 },               // ADD A, B
    [0x81] = { ADD_A_r, REG_C, REG_NONE, 4 },               // ADD A, C
    [0x82] = { ADD_A_r, REG_D, REG_NONE, 4 },               // ADD A, D
    [0x83] = { ADD_A_r, REG_E, REG_NONE, 4 },               // ADD A, E
    [0x84] = { ADD_A_r, REG_H, REG_NONE, 4 },               // ADD A, H
    [0x85] = { ADD_A_r, REG_L, REG_NONE, 4 },               // ADD A, L
    [0x86] = { ADD_A_mHL, REG_NONE, REG_NONE, 8 },          // ADD A, (HL)
    [0x87] = { ADD_A_r, REG_A, REG_NONE, 4 },               // ADD A, A
    [0x88] = { ADC_A_r, REG_B, REG_NONE, 4 },               // ADC A, B
    [0x89] = { ADC_A_r, REG_C, REG_NONE, 4 },               // ADC A, C
    [0x8A] = { ADC_A_r, REG_D, REG_NONE, 4 },               // ADC A, D
    [0x8B] = { ADC_A_r, REG_E, REG_NONE, 4 },               // ADC A, E
    [0x8C] = { ADC_A_r, REG_H, REG_NONE, 4 },               // ADC A, H
    [0x8D] = { ADC_A_r, REG_L, REG_NONE, 4 },               // ADC A, L
    [0x8E] = { ADC_A_mHL, REG_NONE, REG_NONE, 8 },          // ADC A, (HL)
    [0x8F] = { ADC_A_r, REG_A, REG_NONE, 4 },               // ADC A, A
    [0x90] = { SUB_A_r, REG_B, REG_NONE, 4 },               // SUB A, B
    [0x91] = { SUB_A_r, REG_C, REG_NONE, 4 },               // SUB A, C
    [0x92] = { SUB_A_r, REG_D, REG_NONE, 4 },               // SUB A, D
    [0x93] = { SUB_A_r, REG_E, REG_NONE, 4 },               // SUB A, E
    [0x94] = { SUB_A_r, REG_H, REG_NONE, 4 },               // SUB A, H
    [0x95] = { SUB_A_r, REG_L, REG_NONE, 4 },               // SUB A, L
    [0x96] = { SUB_A_mHL, REG_NONE, REG_NONE, 8 },          // SUB A, (HL)
    [0x97] = { SUB_A_r, REG_A, REG_NONE, 4 },               // SUB A, A
    [0x98] = { SBC_A_r, REG_B, REG_NONE, 4 },               // SBC A, B
    [0x99] = { SBC_A_r, REG_C, REG_NONE, 4 },               // SBC A, C
    [0x9A] = { SBC_A_r, REG_D, REG_NONE, 4 },               // SBC A, D
    [0x9B] = { SBC_A_r, REG_E, REG_NONE, 4 },               // SBC A, E
    [0x9C] = { SBC_A_r, REG_H, REG_NONE, 4 },               // SBC A, H
    [0x9D] = { SBC_A_r, REG_L, REG_NONE, 4 },               // SBC A, L
    [0x9E] = { SBC_A_mHL, REG_NONE, REG_NONE, 8 },          // SBC A, (HL)
    [0x9F] = { SBC_A_r, REG_A, REG_NONE, 4 },               // SBC A, A
    [0xA0] = { AND_A_r, REG_B, REG_NONE, 4 },               // AND B
    [0xA1] = { AND_A_r, REG_C, REG_NONE, 4 },               // AND C
    [0xA2] = { AND_A_r, REG_D, REG_NONE, 4 },               // AND D
    [0xA3] = { AND_A_r, REG_E, REG_NONE, 4 },               // AND E
    [0xA4] = { AND_A_r, REG_H, REG_NONE, 4 },               // AND H
    [0xA5] = { AND_A_r, REG_L, REG_NONE, 4 },               // AND L
    [0xA6] = { AND_A_mHL, REG_NONE, REG_NONE, 8 },          // AND (HL)
    [0xA7] = { AND_A_r, REG_A, REG_NONE, 4 },               // AND A
    [0xA8] = { XOR_A_r, REG_B, REG_NONE, 4 },               // XOR B
    [0xA9] = { XOR_A_r, REG_C, REG_NONE, 4 },               // XOR C
    [0xAA] = { XOR_A_r, REG_D, REG_NONE, 4 },               // XOR D
    [0xAB] = { XOR_A_r, REG_E, REG_NONE, 4 },               // XOR E
    [0xAC] = { XOR_A_r, REG_H, REG_NONE, 4 },               // XOR H
    [0xAD] = { XOR_A_r, REG_L, REG_NONE, 4 },               // XOR L
    [0xAE] = { XOR_A_mHL, REG_NONE, REG_NONE, 8 },          // XOR (HL)
    [0xAF] = { XOR_A_r, REG_A, REG_NONE, 4 },               // XOR A
    [0xB0] = { OR_A_r, REG_B, REG_NONE, 4 },                // OR B
    [0xB1] = { OR_A_r, REG_C, REG_NONE, 4 },                // OR C
    [0xB2] = { OR_A_r, REG_D, REG_NONE, 4 },                // OR D
    [0xB3] = { OR_A_r, REG_E, REG_NONE, 4 },                // OR E
    [0xB4] = { OR_A_r, REG_H, REG_NONE, 4 },                // OR H
    [0xB5] = { OR_A_r, REG_L, REG_NONE, 4 },                // OR L
    [0xB6] = { OR_A_mHL, REG_NONE, REG_NONE, 8 },           // OR (HL)
    [0xB7] = { OR_A_r, REG_A, REG_NONE, 4 },                // OR A
    [0xB8] = { CP_A_r, REG_B, REG_NONE, 4 },                // CP B
    [0xB9] = { CP_A_r, REG_C, REG_NONE, 4 },                // CP C
    [0xBA] = { CP_A_r, REG_D, REG_NONE, 4 },                // CP D
    [0xBB] = { CP_A_r, REG_E, REG_NONE, 4 },                // CP E
    [0xBC] = { CP_A_r, REG_H, REG_NONE, 4 },                // CP H
    [0xBD] = { CP_A_r, REG_L, REG_NONE, 4 },                // CP L
    [0xBE] = { CP_A_mHL, REG_NONE, REG_NONE, 8 },           // CP (HL)
    [0xBF] = { CP_A_r, REG_A, REG_NONE, 4 },                // CP A
    [0xC6] = { ADD_A_d8, REG_NONE, REG_NONE, 8 },           // ADD A, d8
    [0xCE] = { ADC_A_d8, REG_NONE, REG_NONE, 8 },           // ADC A, d8
    [0xD6] = { SUB_A_d8, REG_NONE, REG_NONE, 8 },           // SUB A, d8
    [0xDE] = { SBC_A_d8, REG_NONE, REG_NONE, 8 },           // SBC A, d8
    [0xE6] = { AND_A_d8, REG_NONE, REG_NONE, 8 },           // AND d8
    [0xEE] = { XOR_A_d8, REG_NONE, REG_NONE, 8 },           // XOR d8
    [0xF6] = { OR_A_d8, REG_NONE, REG_NONE, 8 },            // OR d8
    [0xFE] = { CP_A_d8, REG_NONE, REG_NONE, 8 }             // CP d8
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
