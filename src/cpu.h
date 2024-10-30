#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef struct {
    uint8_t a, f;  // Accumulator & Flags
    uint8_t b, c, d, e, h, l;  // General-purpose registers
    uint16_t sp, pc;  // Stack Pointer & Program Counter

    uint8_t ime;  // Interrupt Master Enable flag
    uint8_t halted;  // Halt state
} CPU;

typedef void (*InstructionFP)(CPU *cpu, uint8_t *memory);

void initCPU(CPU *cpu);
void executeNextInstruction(CPU *cpu, uint8_t *memory); 

#endif
