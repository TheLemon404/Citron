#pragma once

#include <cstdint>
#include <webgpu/webgpu.hpp>
namespace CitronGraphics {
struct GPUBuffer {
	~GPUBuffer() {
		if (buffer)
			buffer.release();
	}

	wgpu::Buffer buffer;
	wgpu::BufferUsage usage;
	uint32_t size;
	uint32_t entryCount;
};
} // namespace CitronGraphics
