#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "timer.h"

struct CPU;

typedef enum {
    REG_B, REG_C, REG_D, REG_E, REG_H, REG_L, REG_A, REG_NONE
} Register;

typedef struct Instruction {
    void (*execute)(struct CPU *cpu, uint8_t *memory, Register reg1, Register reg2);
    Register reg1;
    Register reg2;
    uint8_t cycles;
} Instruction;

typedef struct CPU {
    uint8_t a, f;              // Accumulator & Flags
    uint8_t b, c, d, e, h, l;  // General-purpose registers
    uint16_t sp, pc;           // Stack Pointer & Program Counter
    uint8_t ime;               // Interrupt Master Enable flag
    uint8_t halted;            // Halt state
    Timer timer;               // Timer for tracking cycles
} CPU;

void initCPU(CPU *cpu);
void executeNextInstruction(CPU *cpu, uint8_t *memory);

#endif
