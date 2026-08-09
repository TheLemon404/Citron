#pragma once

#include "buffer.hpp"
#include "citron_exports.hpp"
#include <lang.hpp>
#include "logger.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <variant>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/fwd.hpp"
#include <glm/gtx/hash.hpp>

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

struct CITRON_GRAPHICS_API DrawUniforms {
	glm::mat4 viewProjection = glm::identity<glm::mat4>();
	glm::vec4 sunLight = glm::vec4(1.0f);
	glm::vec4 sunLightColor = glm::vec4(1.0f);
	glm::vec4 ambientLight = glm::vec4(0.2f, 0.2f, 0.25f, 0.0f);

	bool operator==(const DrawUniforms &other) const {
		return viewProjection == other.viewProjection && sunLight == other.sunLight && sunLightColor == other.sunLightColor && ambientLight == other.ambientLight;
	}
};

struct CITRON_GRAPHICS_API PipelineKey {
	std::shared_ptr<Shader> shader;
	const std::vector<wgpu::TextureFormat> colorAttachmentFormats;
	bool hasDepthStencilAttachment = false;

	bool operator==(const PipelineKey &other) const {
		if (shader == other.shader && colorAttachmentFormats.size() == other.colorAttachmentFormats.size() && hasDepthStencilAttachment == other.hasDepthStencilAttachment) {
			for (size_t i = 0; i < colorAttachmentFormats.size(); i++) {
				if (colorAttachmentFormats[i] != other.colorAttachmentFormats[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
};

struct CITRON_GRAPHICS_API BindGroupEntry {
	uint32_t binding;

	// IN THE FUTURE: std::variant<wgpu::Buffer, wgpu::TextureView, wgpu::Sampler> resource;
	std::variant<wgpu::Buffer, wgpu::TextureView> resource;
	uint64_t offset;
	size_t size = WGPU_WHOLE_SIZE;

	bool operator==(const BindGroupEntry &other) const {
		if (binding == other.binding && resource.index() == other.resource.index() && offset == other.offset && size == other.size) {
			switch (resource.index()) {
			case 0: {
				return static_cast<WGPUBuffer>(std::get<0>(resource)) == static_cast<WGPUBuffer>(std::get<0>(other.resource));
			}
			case 1: {
				return static_cast<WGPUTextureView>(std::get<1>(resource)) == static_cast<WGPUTextureView>(std::get<1>(other.resource));
			}

			default:
				return false;
			}
		}
		return false;
	}
};

struct CITRON_GRAPHICS_API BindGroupKey {
	wgpu::BindGroupLayout layout;
	std::vector<BindGroupEntry> entries = {};

	bool operator==(const BindGroupKey &other) const {
		if (layout == other.layout) {
			if (entries.size() != other.entries.size()) {
				return false;
			}
			for (size_t i = 0; i < entries.size(); i++) {
				if (entries[i] != other.entries[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
};

} // namespace CitronGraphics

// TODO: Test for hash collisions... implement better has functions
namespace std {
template <>
struct hash<CitronGraphics::BindGroupKey> {
	std::size_t operator()(const CitronGraphics::BindGroupKey &key) const {
		size_t seed = 0;
		seed = Hashing::hash_combine(seed, key.layout);
		for (const auto &entry : key.entries) {
			seed = Hashing::hash_combine(seed, entry.binding);
			seed = Hashing::hash_combine(seed, entry.offset);
			seed = Hashing::hash_combine(seed, entry.resource.index());
			seed = Hashing::hash_combine(seed, entry.size);
		}
		return seed;
	}
};

template <>
struct hash<CitronGraphics::PipelineKey> {
	std::size_t operator()(const CitronGraphics::PipelineKey &key) const {
		size_t seed = 0;
		seed = Hashing::hash_combine(seed, hash<std::shared_ptr<CitronGraphics::Shader>>()(key.shader));
		for (const auto &format : key.colorAttachmentFormats) {
			seed = Hashing::hash_combine(seed, format.m_raw);
		}
		return seed;
	}
};

} // namespace std

namespace CitronGraphics {

class CITRON_GRAPHICS_API RendererResourceManager {
  public:
	RendererResourceManager(Device &device) : device(device) {}
	void initResources();
	void releaseUnusedBindGroups();
	GPUBuffer &getEntityModelUniformBuffer(uint32_t entityUUID, ModelUniforms &modelUniforms);
	GPUBuffer &getDrawUniformBuffer(uint16_t renderCount);
	void releaseResources();
	wgpu::BindGroup getBindGroup(BindGroupKey key);
	std::shared_ptr<Pipeline> getPipeline(PipelineKey key);

	std::unordered_map<PipelineKey, std::shared_ptr<Pipeline>> pipelineCache;

  private:
	std::unordered_set<BindGroupKey> usedBindGroupKeysThisFrame;
	std::unordered_map<BindGroupKey, wgpu::BindGroup> bindGroupCache;
	std::unordered_map<uint16_t, GPUBuffer> drawUniformBufferCache;
	std::unordered_map<uint32_t, GPUBuffer> entityModelUniformBufferCache;

	Device &device;
};

} // namespace CitronGraphics
