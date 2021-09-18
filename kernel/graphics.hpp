#pragma once

#include "frame_buffer_config.hpp"
#include <cstdint>
#include <cstdio>



struct PixelColor {
  uint8_t r, g, b;
};

PixelColor ToColor(uint32_t c);

template <typename T>
struct Vector2D {
	T x,y;
};


class PixelWriter {
	public:
		PixelWriter(const FrameBufferConfig& config) : config_{config} {}
		virtual ~PixelWriter() = default;
		uint8_t* PixelAt(Vector2D<int> pos);
		virtual void Write(Vector2D<int> pos, const PixelColor& c) = 0;
		const FrameBufferConfig& config_;
		void CopyTo(PixelWriter& writer, Vector2D<int> size, Vector2D<int> pos);

};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
	public:
		using PixelWriter::PixelWriter;
		void Write(Vector2D<int> pos, const PixelColor& c) override;
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
	public:
		using PixelWriter::PixelWriter;
		void Write(Vector2D<int> pos, const PixelColor& c) override;
};

void FillRectangle(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size, const PixelColor& c);

void WriteAscii(PixelWriter& writer, Vector2D<int> pos, char c, const PixelColor& color);

class console {
	public:
		console(PixelWriter& writer);
		console(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size);
		~console(void) = default;
		void ShowCursor(void);
		void HideCursor(void);
		void ToggleCursor(void);
		void Newline(void);
		void PutString(const char* s);

	private:
		PixelWriter& writer;
		Vector2D<int> cursor;
		const Vector2D<int> pos;
		const Vector2D<int> size;
		bool cursor_visible;
};

int printk(const char* format, ...);



void WriteString(PixelWriter& writer, Vector2D<int> pos, const char* s, const PixelColor& color);
void InitGraphics(const FrameBufferConfig& config);
