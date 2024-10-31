#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef struct {
    uint32_t cycleCount;  
} Timer;

void initTimer(Timer *timer);
void addCycles(Timer *timer, uint8_t cycles);

#endif 
