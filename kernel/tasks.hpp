#pragma once

#include "message.hpp"
#include "queue.hpp"

#include <array>
#include <numeric>
#include <vector>
#include <deque>


const int TaskTimerPeriod = 10;

using TaskFunc = void (uint64_t);

struct TaskContext {
  uint64_t cr3, rip, rflags, reserved1; // offset 0x00
  uint64_t cs, ss, fs, gs; // offset 0x20
  uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp; // offset 0x40
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15; // offset 0x80
  std::array<uint8_t, 512> fxsave_area; // offset 0xc0
} __attribute__((packed));


class Task{
    public:
    Task(int level = 3, uint64_t stack_size = 4096);
    void InitKernelContext(TaskFunc* f, uint64_t arg);
    void InitAppContext(TaskFunc* f, uint64_t arg);
    int SetLevel(int);
    void Load(void);
    void CopyContext(TaskContext& context);
    void WakeUp(void);
    void Sleep(void);
    int ShowLevel(void){
      return this->level;
    }
    bool IsRunning(void){
      return this->isrunning;
    }
    ringqueue<Message, 32> msg_queue;
    int id;

    private:
    alignas(16) TaskContext context;
    int level;
    bool isrunning;
      std::vector<uint64_t> stack;
};

Task* CurrentTask(void);

void LoadNextContext(TaskContext& current_context);

void InitTasks(void);