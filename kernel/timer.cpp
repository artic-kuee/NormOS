#include "timer.hpp"
#include "message.hpp"
#include "tasks.hpp"
#include "queue.hpp"


#include <queue>
#include <deque>


namespace{
    volatile uint32_t& lvt_timer = *reinterpret_cast<uint32_t*>(0xfee00320);
    volatile uint32_t& initial_count = *reinterpret_cast<uint32_t*>(0xfee00380);
    volatile uint32_t& current_count = *reinterpret_cast<uint32_t*>(0xfee00390);
    volatile uint32_t& devide_config = *reinterpret_cast<uint32_t*>(0xfee003e0);

    unsigned long timer_count = 0;
    int task_timer_count = 0;
    priqueue<Timer> timers_{};
}


void InitTimer(void){
    const uint32_t kCountMax = 0x8ffffffu;
    devide_config = 0b1011;
    lvt_timer = 0b010 << 16 | InterruptVector::kLAPICTimer;
    initial_count = kCountMax;
}

unsigned long ShowCount(void){
    return timer_count;
}

void AddTimer(unsigned long timeout, int value, Task* dst ){
    Timer t =Timer(timeout, value, dst);
    timers_.push(Timer(timeout, value, dst));
}

void Tick(TaskContext& current_context){
    timer_count ++;
    task_timer_count ++;
    NotifyEndOfInterrupt();
    while(1){
        if(timers_.empty()){
            break;
        }
        const auto t = timers_.front();
        if(t.timeout != timer_count){
            break;
        } else{
            Message* m = new Message(Message::kTimerTimeout);
            m->arg.timer.value = t.value;
            m->arg.timer.timeout = t.time;
            m->src_task = t.src; 

            t.dst->msg_queue.push(*m);
            t.dst->WakeUp();
            if(t.value > 0){
                AddTimer(t.time, t.value, t.dst);
            }
            timers_.pop();
    }

        
    }
    if(task_timer_count == TaskTimerPeriod){
        task_timer_count = 0;
        LoadNextContext(current_context);
    }
}

