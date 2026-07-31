#pragma once

#include "assets.hpp"
#include "buffer.hpp"
#include "citron_exports.hpp"

#include "device.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "resources.hpp"
#include "shader.hpp"
#include <cstddef>
#include <functional>
#include <layer.hpp>
#include <map>
#include <material.hpp>
#include <memory>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

namespace CitronGraphics {

class CITRON_GRAPHICS_API Frame;

struct CITRON_GRAPHICS_API RenderObject {
	uint64_t entityUUID;
	ModelUniforms modelUniforms;
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

	void drawRenderData(std::vector<RenderableReferenceData> renderableReferenceData);

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

struct RendererContext {
	Device &device;
	AssetManager &assetManager;
	RendererResourceManager &rendererResourcesManager;
};

class CITRON_GRAPHICS_API Renderer {
  public:
	Renderer(Window &window, AssetManager &assetManager) : device(window), assetManager(assetManager), rendererResourcesManager(device) {}

	bool frameReady() { return device.prepareCurrentSurfaceTexture(); }
	Frame beginFrame();
	void endFrame(Frame &frame);

	void init();
	void end();
	void onEvent(Event &e);

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

	std::shared_ptr<Pipeline> getPipeline(PipelineKey key) {
		if (!rendererResourcesManager.pipelineCache.contains(key)) {
			auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), key.shader, key.textureFormat);
			rendererResourcesManager.pipelineCache[key] = pipeline;
		}
		return rendererResourcesManager.pipelineCache[key];
	}

	wgpu::BindGroup getBindGroup(BindGroupKey key) {
		if (!rendererResourcesManager.bindGroupCache.contains(key)) {
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
			rendererResourcesManager.bindGroupCache[key] = bindGroup;
		}
		return rendererResourcesManager.bindGroupCache[key];
	}

	static std::vector<RenderObject> sortByShader(std::vector<RenderObject> &renderables, int start = 0, int end = -1);
	static std::vector<RenderObject> sortByMesh(std::vector<RenderObject> &renderables, int start = 0, int end = -1);
	static std::vector<RenderObject> sortByMaterial(std::vector<RenderObject> &renderables, int start = 0, int end = -1);

	RendererContext getContext() {
		return {
			device,
			assetManager,
			rendererResourcesManager,
		};
	}

	wgpu::Texture &getColorTarget() { return colorTarget; }
	wgpu::TextureView &getColorTargetView() { return colorTargetView; }

  private:
	RendererResourceManager rendererResourcesManager;
	AssetManager &assetManager;

	wgpu::Texture colorTarget;
	wgpu::TextureView colorTargetView;

	Device device;
};

} // namespace CitronGraphics
