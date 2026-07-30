#pragma once

#include "buffer.hpp"
#include "citron_exports.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <map>
#include <variant>
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {

struct CITRON_GRAPHICS_API ModelUniforms {
	glm::mat4 transform = glm::identity<glm::mat4>();
};

struct CITRON_GRAPHICS_API FrameUniforms {
	glm::mat4 viewProjection = glm::identity<glm::mat4>();
};

struct CITRON_GRAPHICS_API PipelineKey {
	std::shared_ptr<Shader> shader;
	wgpu::TextureFormat textureFormat;

	bool operator==(const PipelineKey &other) const {
		return shader == other.shader && textureFormat == other.textureFormat;
	}

	bool operator<(const PipelineKey &other) const {
		return shader < other.shader || (shader == other.shader && textureFormat < other.textureFormat);
	}
};

struct CITRON_GRAPHICS_API BindGroupEntry {
	uint32_t binding;

	// IN THE FUTURE: std::variant<wgpu::Buffer, wgpu::TextureView, wgpu::Sampler> resource;
	std::variant<wgpu::Buffer> resource;
	uint64_t offset;
	size_t size = WGPU_WHOLE_SIZE;

	bool operator==(const BindGroupEntry &other) const {
		return binding == other.binding && resource == other.resource && offset == other.offset && size == other.size;
	}

	bool operator<(const BindGroupEntry &other) const {
		if (binding != other.binding)
			return binding < other.binding;
		if (offset != other.offset)
			return offset < other.offset;
		if (size != other.size)
			return size < other.size;

		// Extract raw underlying C pointers for stable handle ordering
		auto thisPtr = static_cast<WGPUBuffer>(std::get<wgpu::Buffer>(resource));
		auto otherPtr = static_cast<WGPUBuffer>(std::get<wgpu::Buffer>(other.resource));
		return thisPtr < otherPtr;
	}
};

struct CITRON_GRAPHICS_API BindGroupKey {
	wgpu::BindGroupLayout layout;
	std::vector<BindGroupEntry> entries = {};

	bool operator==(const BindGroupKey &other) const {
		return layout == other.layout && entries == other.entries;
	}

	bool operator<(const BindGroupKey &other) const {
		// Compare raw layout handles
		auto thisLayoutPtr = static_cast<WGPUBindGroupLayout>(layout);
		auto otherLayoutPtr = static_cast<WGPUBindGroupLayout>(other.layout);
		if (thisLayoutPtr != otherLayoutPtr) {
			return thisLayoutPtr < otherLayoutPtr;
		}

		// Deep array comparison instead of just checking array size
		return std::lexicographical_compare(
			entries.begin(), entries.end(),
			other.entries.begin(), other.entries.end());
	}
};

class CITRON_GRAPHICS_API RendererResourceManager {
  public:
	RendererResourceManager(Device &device) : device(device) {}
	void initResources();
	GPUBuffer &getEntityModelUniformBuffer(uint64_t entityUUID, ModelUniforms &modelUniforms, bool isDirty = false);

	std::map<PipelineKey, std::shared_ptr<Pipeline>> pipelineCache;
	std::map<BindGroupKey, wgpu::BindGroup> bindGroupCache;
	std::map<uint64_t, GPUBuffer> entityModelUniformBufferCache;

	FrameUniforms frameUniforms;
	GPUBuffer frameUniformBuffer;

  private:
	Device &device;
};

} // namespace CitronGraphics
