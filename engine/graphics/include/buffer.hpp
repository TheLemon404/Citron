#pragma once

#include <cstdint>
#include <webgpu/webgpu.hpp>
namespace CitronGraphics {
struct GPUBuffer {
	wgpu::Buffer buffer;
	wgpu::BufferUsage usage;
	uint32_t size;
};
} // namespace CitronGraphics
