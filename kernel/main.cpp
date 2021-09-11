#include "graphics.hpp"
#include "asmfunc.h"
#include "segment.hpp"
#include "memorymanager.hpp"
#include "memory_map.hpp"

#include "asmfunc.h"

alignas(16) uint8_t kernel_main_stack[1024*1024];
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)]; 
PixelWriter* pixel_writer;
char main_consule_buf[sizeof(consule)];
consule* main_consule;

extern "C" void KernelMainNewStack(const FrameBufferConfig& frame_buffer_config_ref, const MemoryMap& memory_map_ref) {
	InitGraphics(frame_buffer_config_ref);
	main_consule->PutString("hello world\n NormOS");
	SetupSegment();
	SetupIPageTable();
	SetupMemoryManager(memory_map_ref);
	SetupHeap();
	main_consule->ShowCursor();
	while(1);
}



