#include "timer.h"

void initTimer(Timer *timer) {
    timer->cycleCount = 0;
}

void addCycles(Timer *timer, uint8_t cycles) {
    timer->cycleCount += cycles;
}
