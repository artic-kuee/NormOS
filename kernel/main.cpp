#include "graphics.hpp"
#include "asmfunc.h"
#include "segment.hpp"

#include "asmfunc.h"

alignas(16) uint8_t kernel_main_stack[1024*1024];
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)]; 
PixelWriter* pixel_writer;
char main_consule_buf[sizeof(consule)];
consule* main_consule;

extern "C" void kernelmain(const FrameBufferConfig& frame_buffer_config_ref) {
	SetStack(*kernel_main_stack);
	SetupSegment();
//	SetupMemoryManager(memorymap_ref);
	InitGraphics(frame_buffer_config_ref);
	main_consule->PutString("hello world\n");
	//WriteString(*pixel_writer, Vector2D<int>{100,100},  "Hello World", {255,255,255});
	while(1);
}



