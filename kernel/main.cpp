#include "graphics.hpp"
#include "asmfunc.h"
#include "segment.hpp"
#include "interrupt.hpp"
#include "memorymanager.hpp"


#include "memory_map.hpp"
#include "message.hpp"
#include "timer.hpp"
#include "tasks.hpp"
#include "acpi.hpp"
#include "queue.hpp"

#include "asmfunc.h"

#include <cstdint>
#include <cstddef>
#include <cstdio>

#include <numeric>
#include <vector>
#include <deque>
#include <limits>




alignas(16) uint8_t kernel_main_stack[1024*1024];
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)]; 
PixelWriter* pixel_writer;
char main_console_buf[sizeof(console)];
console* main_console;
std::deque<Message>* msg_queue;
Task* main_task;
Task* TerminalTask;


void TerminalTaskFunc(uint64_t i){
	while(1);
	Task* t = CurrentTask();
	//AddTimer(30,1,t);
	while(!t->msg_queue.empty()){
			printk("subtask\n");
		t->msg_queue.pop();
	}
	printk("sleep context\n");
	t->Sleep();
}



extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config_ref, const MemoryMap& memory_map_ref, const acpi::RSDP& acpi_table, void* volume_image) {
	DisableInt();
	SetStack();
	InitGraphics(frame_buffer_config_ref);
	printk("hello world\n NormOS\n");
	SetupSegment();
	SetupIPageTable();
	SetupMemoryManager(memory_map_ref);
	if(SetupHeap()){
		printk("Heap Error\n");
	}

	InitInterrupt();
	::msg_queue = new std::deque<Message>;
	main_console->ShowCursor();
	InitTasks();
	printk("maintask %d\n",CurrentTask()->id);
	::TerminalTask = new Task(0);
	TerminalTask->InitKernelContext(TerminalTaskFunc, 0);
	TerminalTask->WakeUp();
	printk("main task\n");
	AddTimer(50, 3, CurrentTask());
	while(1){
		if(!(CurrentTask()->msg_queue.empty())){
			main_console->ToggleCursor();
			printk("msg_queue,%d\n", CurrentTask()->msg_queue.size());
			CurrentTask()->msg_queue.pop();	
		}
		
	};
}


