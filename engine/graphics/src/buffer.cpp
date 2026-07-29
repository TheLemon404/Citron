#include "buffer.hpp"

using namespace CitronGraphics;

GPUBuffer::~GPUBuffer() {
	if (buffer)
		buffer.release();
}
