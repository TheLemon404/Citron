#pragma once

#include "citron_exports.hpp"
#include <cstdint>
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {
struct CITRON_GRAPHICS_API GPUBuffer {
	wgpu::Buffer buffer;
	uint32_t size;
	uint32_t entryCount;

	bool operator==(const GPUBuffer &other) const {
		return buffer == other.buffer && size == other.size && entryCount == other.entryCount;
	}
};
} // namespace CitronGraphics
