#pragma once

#include "interrupt.hpp"
#include "tasks.hpp"
#include "graphics.hpp"

unsigned long ShowCount(void);

struct Timer {
    public:
    Timer(unsigned long time, int value, Task* dst): timeout{(time + ShowCount())}, time{time}, value{value}, dst{dst}, src{CurrentTask()} {}
    ~Timer(void) = default;
    unsigned long timeout;
    unsigned long time;
    int value;
    Task* dst;
    Task* src;

};

inline bool operator<(const Timer& lhs, const Timer& rhs) {
    return (rhs.timeout - ShowCount()) < (lhs.timeout - ShowCount());
}

void InitTimer(void);


void AddTimer(unsigned long timeout, int value, Task* dst);

extern "C" void Tick(TaskContext& current_context);

extern "C" void hoge();
