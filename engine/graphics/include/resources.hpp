#pragma once

#include "buffer.hpp"
#include "citron_exports.hpp"
#include "glm/fwd.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <variant>
#include <webgpu/webgpu.hpp>

constexpr uint32_t MAX_MODEL_UNIFORMS = 1000;

namespace CitronGraphics {

struct CITRON_GRAPHICS_API RenderableReferenceData {
	uint32_t entityUUID;
	glm::mat4 transform;
	uint32_t meshUUID;
	uint32_t materialUUID;
};

struct CITRON_GRAPHICS_API ModelUniforms {
	glm::mat4 transform = glm::identity<glm::mat4>();
};

struct CITRON_GRAPHICS_API FrameUniforms {
	glm::mat4 viewProjection = glm::identity<glm::mat4>();
	glm::vec4 sunLight = glm::vec4(1.0f);
	glm::vec4 sunLightColor = glm::vec4(1.0f);
	glm::vec4 ambientLight = glm::vec4(0.2f, 0.2f, 0.25f, 0.0f);

	bool operator==(const FrameUniforms &other) const {
		return viewProjection == other.viewProjection && sunLight == other.sunLight && sunLightColor == other.sunLightColor && ambientLight == other.ambientLight;
	}
};

struct CITRON_GRAPHICS_API PipelineKey {
	std::shared_ptr<Shader> shader;
	const std::vector<wgpu::TextureFormat> &colorAttachmentFormats;
	bool hasDepthStencilAttachment = false;

	bool operator==(const PipelineKey &other) const {
		return shader == other.shader && colorAttachmentFormats.size() == other.colorAttachmentFormats.size() && hasDepthStencilAttachment == other.hasDepthStencilAttachment;
	}
};

struct CITRON_GRAPHICS_API BindGroupEntry {
	uint32_t binding;

	// IN THE FUTURE: std::variant<wgpu::Buffer, wgpu::TextureView, wgpu::Sampler> resource;
	std::variant<wgpu::Buffer, wgpu::TextureView> resource;
	uint64_t offset;
	size_t size = WGPU_WHOLE_SIZE;

	bool operator==(const BindGroupEntry &other) const {
		return binding == other.binding && resource == other.resource && offset == other.offset && size == other.size;
	}
};

struct CITRON_GRAPHICS_API BindGroupKey {
	wgpu::BindGroupLayout layout;
	std::vector<BindGroupEntry> entries = {};

	bool operator==(const BindGroupKey &other) const {
		return layout == other.layout && entries == other.entries;
	}
};

} // namespace CitronGraphics

// TODO: Test for hash collisions... implement better has functions
namespace std {
template <>
struct hash<CitronGraphics::BindGroupKey> {
	std::size_t operator()(const CitronGraphics::BindGroupKey &key) const {
		size_t resourceHash = hash<size_t>()(key.entries[0].resource.index());
		size_t resourceNumHash = hash<size_t>()(key.entries.size());
		return resourceHash ^ (resourceNumHash << 1);
	}
};

template <>
struct hash<CitronGraphics::PipelineKey> {
	std::size_t operator()(const CitronGraphics::PipelineKey &key) const {
		size_t vertexShaderHash = hash<std::shared_ptr<CitronGraphics::Shader>>()(key.shader);
		size_t fragmentShaderHash = hash<wgpu::TextureFormat::W>()(key.colorAttachmentFormats[0].m_raw);
		return vertexShaderHash ^ (fragmentShaderHash << 1);
	}
};
} // namespace std

namespace CitronGraphics {

class CITRON_GRAPHICS_API RendererResourceManager {
  public:
	RendererResourceManager(Device &device) : device(device) {}
	void initResources();
	GPUBuffer &getEntityModelUniformBuffer(uint32_t entityUUID, ModelUniforms &modelUniforms, bool isDirty = false);

	std::unordered_map<PipelineKey, std::shared_ptr<Pipeline>> pipelineCache;
	std::unordered_map<BindGroupKey, wgpu::BindGroup> bindGroupCache;
	std::unordered_map<uint32_t, GPUBuffer> entityModelUniformBufferCache;

	FrameUniforms frameUniforms;
	GPUBuffer frameUniformBuffer;

	GPUBuffer modelUniformsBuffer;

  private:
	Device &device;
};

} // namespace CitronGraphics
