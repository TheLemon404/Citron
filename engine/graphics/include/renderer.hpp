#pragma once

#include "assets.hpp"
#include "buffer.hpp"
#include "citron_exports.hpp"

#include "device.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <cstddef>
#include <functional>
#include <layer.hpp>
#include <map>
#include <material.hpp>
#include <memory>
#include <unordered_map>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

namespace CitronGraphics {

struct CITRON_GRAPHICS_API FrameUniforms {
	glm::mat4 mvp = glm::identity<glm::mat4>();
};

class CITRON_GRAPHICS_API Frame;

struct CITRON_GRAPHICS_API RenderObject {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	std::shared_ptr<Shader> shader;
};

class CITRON_GRAPHICS_API Renderer;

class CITRON_GRAPHICS_API RenderPass {
  public:
	RenderPass(Renderer &renderer, wgpu::Texture &targetTexture,
			   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame);
	~RenderPass();
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass(RenderPass &&) = default;

	void drawRenderData(std::vector<uint64_t> meshUUIDs, std::vector<uint64_t> materialUUIDs);

	void setPipeline(std::shared_ptr<Pipeline> pipeline);
	void setMesh(std::shared_ptr<Mesh> geometry);
	void setBindGroup(int index, wgpu::BindGroup bindGroup);
	void draw(std::shared_ptr<Mesh> geometry);
	void end();

	wgpu::RenderPassEncoder &getRenderPassEncoder() {
		return renderPassEncoder;
	}

	wgpu::TextureView &getTargetView() { return targetView; }

	Frame &getParentFrame() { return parentFrame; }

  private:
	Renderer &renderer;
	Frame &parentFrame;
	wgpu::Texture &targetTexture;
	wgpu::CommandEncoder &commandEncoder;
	wgpu::TextureView targetView;
	wgpu::RenderPassEncoder renderPassEncoder;
};

class CITRON_GRAPHICS_API Frame {
  public:
	Frame(Renderer &renderer, wgpu::CommandEncoder encoder,
		  wgpu::SurfaceTexture &surfaceTexture);

	RenderPass beginRenderPass(wgpu::Texture &tartetTexture);

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	wgpu::CommandEncoder &getEncoder() { return encoder; }

  private:
	Renderer &renderer;
	wgpu::CommandEncoder encoder;
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

struct RendererContext {
	Device &device;
	FrameUniforms &frameUniforms;
	AssetManager &assetManager;
	std::map<PipelineKey, std::shared_ptr<Pipeline>> &pipelineCache;
	std::map<BindGroupKey, wgpu::BindGroup> &bindGroupCache;
};

class CITRON_GRAPHICS_API Renderer {
  public:
	Renderer(Window &window, AssetManager &assetManager) : device(window), assetManager(assetManager) {}

	bool frameReady() { return device.prepareCurrentSurfaceTexture(); }
	Frame beginFrame();
	void endFrame(Frame &frame);

	void init();
	void end();
	void onEvent(Event &e);

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

	std::shared_ptr<Pipeline> getPipeline(PipelineKey key) {
		if (!pipelineCache.contains(key)) {
			auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), key.shader, key.textureFormat);
			pipelineCache[key] = pipeline;
		}
		return pipelineCache[key];
	}

	wgpu::BindGroup getBindGroup(BindGroupKey key) {
		if (!bindGroupCache.contains(key)) {
			std::vector<wgpu::BindGroupEntry> entries;
			for (const auto &e : key.entries) {
				wgpu::BindGroupEntry entry = {};
				entry.setDefault();
				entry.binding = e.binding;
				entry.buffer = std::get<wgpu::Buffer>(e.resource);
				entry.offset = e.offset;
				entry.size = e.size;
				entries.push_back(entry);
			}
			wgpu::BindGroupDescriptor bindGroupDesc = {};
			bindGroupDesc.setDefault();
			bindGroupDesc.label = wgpu::StringView("test");
			bindGroupDesc.nextInChain = nullptr;
			bindGroupDesc.entryCount = entries.size();
			bindGroupDesc.entries = entries.data();
			bindGroupDesc.layout = key.layout;
			wgpu::BindGroup bindGroup = device.getWGPUDevice().createBindGroup(bindGroupDesc);
			bindGroupCache[key] = bindGroup;
		}
		return bindGroupCache[key];
	}

	GPUBuffer &getFrameUniformBuffer() { return frameUniformBuffer; }

	RendererContext getContext() {
		return {
			device,
			frameUniforms,
			assetManager,
			pipelineCache,
			bindGroupCache,
		};
	}

	wgpu::Texture &getColorTarget() { return colorTarget; }
	wgpu::TextureView &getColorTargetView() { return colorTargetView; }

  private:
	AssetManager &assetManager;

	wgpu::Texture colorTarget;
	wgpu::TextureView colorTargetView;

	Device device;
	std::map<PipelineKey, std::shared_ptr<Pipeline>> pipelineCache;
	std::map<BindGroupKey, wgpu::BindGroup> bindGroupCache;

	// frame uniforms
	FrameUniforms frameUniforms;
	GPUBuffer frameUniformBuffer;
};

} // namespace CitronGraphics
