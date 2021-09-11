#include "graphics.hpp"
//#include "basicfunc.hpp"

#include <vector>
#include <algorithm>

extern const uint8_t _binary_hankaku_bin_start;
extern const uint8_t _binary_hankaku_bin_end;
extern const uint8_t _binary_hankaku_bin_size;

extern char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
extern PixelWriter* pixel_writer;

extern char main_consule_buf[sizeof(consule)];
extern consule* main_consule;


PixelColor Tocolor(uint32_t c){
	return {static_cast<uint8_t>((c>>16)&0xff), static_cast<uint8_t>((c>>8)&0xff), static_cast<uint8_t>(c&0xff)};
}

uint8_t* PixelWriter::PixelAt(Vector2D<int> pos){ 
	return this->config_.frame_buffer + 4 * (this->config_.pixels_per_scan_line * pos.y + pos.x);
}

void PixelWriter::CopyTo(PixelWriter& writer, Vector2D<int> size, Vector2D<int> pos){
	int x_max, y_max;
	if(this->config_.horizontal_resolution - pos.x > size.x){
		x_max = size.x;
	} else {
		x_max = this->config_.horizontal_resolution - pos.x;
	}
	if(this->config_.vertical_resolution - pos.y > size.y){
		y_max = size.y;
	} else {
		y_max = this->config_.vertical_resolution - pos.y;
	}
	for(int i = 0; i < y_max ; i++){
		memcpy(this->PixelAt(Vector2D<int>{0, pos.y + i}), writer.PixelAt(Vector2D<int>{0, i}), x_max);
	}
}

void RGBResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.r;
  p[1] = c.g;
  p[2] = c.b;
}

void BGRResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.b;
  p[1] = c.g;
  p[2] = c.r;
}

void FillRectangle(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size, const PixelColor& c){
	Vector2D<int> temp;
	for(temp.y = pos.y; temp.y < pos.y + size.y; temp.y++){
		for(temp.x = pos.x; temp.x < pos.x + size.x; temp.x++){
			writer.Write(temp, c);
		}
	}
}




namespace {

const uint8_t* GetFont(char c) {
  auto index = 16 * static_cast<unsigned int>(c);
  if (index >= reinterpret_cast<uintptr_t>(&_binary_hankaku_bin_size)) {
    return nullptr;
  }
  return &_binary_hankaku_bin_start + index;
	
}
}



void WriteAscii(PixelWriter& writer, Vector2D<int> pos, char c, const PixelColor& color){
	const uint8_t* font = GetFont(c);
	Vector2D<int> temp;
	for(temp.y = pos.y; temp.y < pos.y + 16; temp.y++){
		for(temp.x = pos.x; temp.x < pos.x + 8; temp.x++){
			if((font[temp.y - pos.y]<<(temp.x - pos.x)) & 0x80u){
				writer.Write(temp, color);
			}
		}
	}
}

consule::consule(PixelWriter& writer) : writer{writer}, pos{Vector2D<int>{0,0}}, size{Vector2D<int>{static_cast<int>(writer.config_.horizontal_resolution),static_cast<int>(writer.config_.vertical_resolution)}}, cursor{Vector2D<int>{0,0}}, cursor_visible{false} {
	FillRectangle(this->writer, this->pos, this->size, {0,0,0});
}

consule::consule(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size) : writer{writer}, pos{pos}, size{size}, cursor{Vector2D<int>{0,0}}, cursor_visible{false} {
	FillRectangle(this->writer, this->pos, this->size, {0,0,0});
}

void consule::ShowCursor(void){
	Vector2D<int> temp;
	temp.x = this->pos.x + this->cursor.x;
	temp.y = this->pos.y + this->cursor.y;
	FillRectangle(this->writer, temp, {8,16}, {0xff,0xff,0xff});
	this->cursor_visible = true;
}

void consule::HideCursor(void){
	Vector2D<int> temp;
	temp.x = this->pos.x + this->cursor.x;
	temp.y = this->pos.y + this->cursor.y;
	FillRectangle(this->writer, temp, {8,16}, {0,0,0});
	this->cursor_visible = false;
}

void consule::ToggleCursor(void){
	if(this->cursor_visible){
		this->HideCursor();
	} else {
		this->ShowCursor();
	}
}

void consule::Newline(void){
	if(this->cursor.y + 32 > this->size.y){
		for(int i = this->pos.y; i < this->cursor.y + 16; i++){
			memcpy(this->writer.PixelAt({0,i}),this->writer.PixelAt({0,i+16}), 4 * this->size.x);
		}
	}else{
		this->cursor.y += 16;
	}
	this->cursor.x =0;
}

void consule::PutString(const char* s){
	this->HideCursor();
	while (*s) {
	    if (*s == '\n' | this->cursor.x + 8 > this->size.x) {
    	  this->Newline();
    	} else {
      		WriteAscii(this->writer, this->cursor, *s, {0xff,0xff,0xff});
      		this->cursor.x += 8;
    	}
    	++s;
	}
}
	

void WriteString(PixelWriter& writer, Vector2D<int> pos, const char* s, const PixelColor& color){
	for(int i = 0; s[i] != '\0'; i++){
		WriteAscii(writer, pos, s[i], color);
		pos.x += 8;

	}
}


void InitGraphics(const FrameBufferConfig& config){
	FrameBufferConfig screen_config = config;
	switch (config.pixel_format) {
		case kPixelRGBResv8BitPerColor:
			pixel_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{config};
			break;

		case kPixelBGRResv8BitPerColor:
			pixel_writer =  new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{config};
			break;
	}
	Vector2D<int> zeros, max;
	zeros.x = 0;
	zeros.y = 0;
	max.x = config.horizontal_resolution;
	max.y = config.vertical_resolution;
	FillRectangle(*pixel_writer, zeros, max, {0,0,0});
	main_consule = new(main_consule_buf)
	consule(*pixel_writer);
	return;
}
