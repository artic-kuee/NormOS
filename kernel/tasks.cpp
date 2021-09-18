#include "tasks.hpp"

#include "message.hpp"
#include "asmfunc.h"
#include "graphics.hpp"

#include <cstdint>


namespace{
    std::array<ringqueue<Task*, 256>, 4> run_queue;
    int nextid = 4;
    Task* current_task;
}

Task::Task(int l, uint64_t stack_size){
    if(l >= 0 && l < 4){
        this->level = l;
    } else{
        this->level = 3;
    }
    this->isrunning = false;
    this->stack.resize(stack_size >> 6);
    this->id = nextid;
    nextid++;
    printk("newtask %d, %d\n", this->id, this->level);
}

void Task::InitKernelContext(TaskFunc* f, uint64_t arg){
    this->context.rip = reinterpret_cast<uint64_t>(f);
    this->context.rsp = (reinterpret_cast<uint64_t>(&(this->stack[0]) + sizeof(stack)) & ~0xflu) - 8;
    this->context.rdi = arg;
    this->context.rflags = 0x202;
    this->context.cs = 1<<3;
    this->context.ss = 2<<3;
    this->context.fs = 0;
    this->context.gs = 0;
    this->context.cr3 = GetCR3();
    *reinterpret_cast<uint32_t*>(&this->context.fxsave_area[24]) = 0x1f80;
    printk("initcontext %d rsp:%u\n", this->id, this->context.rsp);
}

void Task::InitAppContext(TaskFunc* f, uint64_t arg){
    this->context.rip = reinterpret_cast<uint64_t>(f);
    this->context.rsp = (reinterpret_cast<uint64_t>(this->stack[0] + sizeof(this->stack)) & ~0xflu) - 8;
    this->context.rdi = arg;
    this->context.rflags = 0x202;
    this->context.cs = 4<<3;
    this->context.ss = 3<<3;
    this->context.fs = 0;
    this->context.gs = 0;
    *reinterpret_cast<uint32_t*>(&this->context.fxsave_area[24]) = 0x1f80;
}


void Task::Load(void){
    LoadContext(&(this->context));
}






void Task::CopyContext(TaskContext& context){         
    memcpy(&(this->context), &context, sizeof(this->context));
}

int Task::SetLevel(int l){
    if(l >= 0 && l < 4){
        this->level = l;
        return 0;
    }
    return 1;
}

void Task::WakeUp(void){
    if(!this->isrunning){
        run_queue[this->level].push(this);
        this->isrunning = true;
        printk("wakeup %d %d\n", run_queue[this->level].back()->id, run_queue[this->level].back()->level);
    }
    return;
}

void Task::Sleep(void){
    this->isrunning = false;
    if(this == CurrentTask()){
        run_queue[this->level].pop();
        for (int i = 0; i < 3; i++){
            if(!run_queue[i].empty()){
                SwitchContext(run_queue[i].front(), this);
                return;
            }
        }

        while(1);
    }
}


Task* CurrentTask(void){
    return current_task;
}


void LoadNextContext(TaskContext& current_context){
    printk("runqueue size:%d lasttask:%d\n",run_queue[0].size(),run_queue[0].front()->id);
    const auto prev = current_task;
        printk("prev context id:%d level:%d\n", current_task->id,current_task->ShowLevel());
        if(current_task->ShowLevel() > 3 | current_task->ShowLevel() < 0){
            printk("Err: illegal task");
            while(1);
        }
        printk("%u\n", prev);
    prev->CopyContext(current_context);

    if(run_queue[prev->ShowLevel()].empty()){
        printk("err");
        while(1);
    }
    printk("level:%d, %dtasks\n",prev->ShowLevel() ,run_queue[prev->ShowLevel()].size());
    run_queue[prev->ShowLevel()].pop();
    run_queue[prev->ShowLevel()].push(prev);
    printk("pushed task %d\n",run_queue[prev->ShowLevel()].back()->id);
    printk("next task %d,%d",run_queue[0].front()->id,run_queue[0].front()->ShowLevel());
    Task* next = nullptr;
    while(next == nullptr){
        for(int i = 0; i < 4; i++){
            while(!run_queue[i].empty()){
                if(run_queue[i].front()->IsRunning()){
                    next = run_queue[i].front();
                    printk("next task %d,%d\n",run_queue[i].front()->id, run_queue[i].front()->ShowLevel());
                    break;
                } else{
                    run_queue[next->ShowLevel()].pop();
                }
            }
        }
    }

    printk("next context id:%d level:%d\n", next->id, next->ShowLevel());

        printk("%u\n", next);
    current_task = next;
    next->Load();
}

extern Task* main_task;

void InitTasks(void){
    printk("%d\n",run_queue[0].size());
    main_task = new Task(0, 0);
    main_task->WakeUp();
    current_task = main_task;
}