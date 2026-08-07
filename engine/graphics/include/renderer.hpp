#pragma once

#include "assets.hpp"
#include "buffer.hpp"
#include "citron_exports.hpp"

#include "device.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "resources.hpp"
#include "shader.hpp"
#include "view.hpp"
#include <cstddef>
#include <functional>
#include <layer.hpp>
#include <map>
#include <material.hpp>
#include <memory>
#include <set>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

namespace CitronGraphics {

class CITRON_GRAPHICS_API Frame;

struct CITRON_GRAPHICS_API RenderObject {
	uint32_t entityUUID;
	ModelUniforms modelUniforms;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	std::shared_ptr<Shader> shader;
};

class CITRON_GRAPHICS_API Renderer;

struct RenderPassColorAttachment {
	wgpu::Texture targetTexture;
	wgpu::TextureView targetTextureView;
	wgpu::TextureFormat textureFormat = wgpu::TextureFormat::BGRA8UnormSrgb;
	wgpu::Color clearValue = {0.247, 0.247, 0.247, 1.0};
};

struct RenderPassDepthStencilAttachment {
	wgpu::Texture targetTexture;
	wgpu::TextureView targetTextureView;
	float depthClearValue = 1.0f;
	float stencilClearValue = 0.0f;
};

struct RenderPassParams {
	std::vector<RenderPassColorAttachment> colorAttachments;
	bool containsDepthStencil = false;
	RenderPassDepthStencilAttachment depthStencilAttachment;
};

class CITRON_GRAPHICS_API RenderPass {
  public:
	RenderPass(Renderer &renderer, RenderPassParams &params,
			   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame);
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass(RenderPass &&) = default;

	void drawFullscreenQuadPass(std::shared_ptr<Mesh> fullscreenQuad, std::shared_ptr<Shader> shader, RenderPass &renderPass);
	void drawRenderData(std::vector<RenderObject> renderableReferenceData, RenderPass &renderPass);

	void setPipeline(std::shared_ptr<Pipeline> pipeline);
	void setMesh(std::shared_ptr<Mesh> geometry);
	void setBindGroup(int index, wgpu::BindGroup bindGroup);
	void draw(std::shared_ptr<Mesh> geometry);
	void end();

	wgpu::RenderPassEncoder &getRenderPassEncoder() {
		return renderPassEncoder;
	}

	wgpu::TextureView &getColorTargetView(int attachmentIndex) { return params.colorAttachments[attachmentIndex].targetTextureView; }

	Frame &getParentFrame() { return parentFrame; }

	const std::vector<RenderPassColorAttachment> &getColorAttachments() const { return params.colorAttachments; }
	const std::vector<wgpu::TextureFormat> &getColorAttachmentFormats() const { return colorAttachmentFormats; }

	const RenderPassParams &getParams() const { return params; }

  private:
	std::vector<wgpu::TextureFormat> colorAttachmentFormats;
	std::vector<wgpu::RenderPassColorAttachment> renderPassColorAttachments;
	Renderer &renderer;
	Frame &parentFrame;
	RenderPassParams &params;
	wgpu::CommandEncoder &commandEncoder;
	wgpu::RenderPassEncoder renderPassEncoder;
};

class CITRON_GRAPHICS_API Frame {
  public:
	Frame(Renderer &renderer, wgpu::CommandEncoder encoder,
		  wgpu::SurfaceTexture &surfaceTexture, View &view);

	RenderPass beginRenderPass(RenderPassParams &params);

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	wgpu::CommandEncoder &getEncoder() { return encoder; }

	View &getView() { return view; }

  private:
	View &view;
	Renderer &renderer;
	wgpu::CommandEncoder encoder;
};

struct RendererContext {
	Device &device;
	AssetManager &assetManager;
	RendererResourceManager &rendererResourcesManager;
};

struct RenderObjectCache {
	std::set<uint32_t> entityUUIDs;
	std::vector<RenderObject> renderObjects;
};

class CITRON_GRAPHICS_API Renderer {
  public:
	Renderer(Window &window, AssetManager &assetManager) : window(window), device(window), assetManager(assetManager), rendererResourcesManager(device) {}

	bool frameReady() { return device.prepareCurrentSurfaceTexture(); }
	Frame beginFrame(View &view);
	void endFrame(Frame &frame);

	void render(Frame &frame, std::vector<RenderableReferenceData> renderableReferenceData, glm::ivec2 iviewportSize);

	void init();
	void end();
	void onEvent(Event &e);

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

	std::shared_ptr<Pipeline> getPipeline(PipelineKey key) {
		if (!rendererResourcesManager.pipelineCache.contains(key)) {
			auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), key.colorAttachmentFormats, key.hasDepthStencilAttachment, key.shader);
			rendererResourcesManager.pipelineCache[key] = pipeline;
		}
		return rendererResourcesManager.pipelineCache[key];
	}

	wgpu::BindGroup getBindGroup(BindGroupKey key) {
		if (!rendererResourcesManager.bindGroupCache.contains(key)) {
			std::vector<wgpu::BindGroupEntry> entries;
			for (const auto &e : key.entries) {
				switch (e.resource.index()) {
				case 0: {
					wgpu::BindGroupEntry entry = {};
					entry.setDefault();
					entry.binding = e.binding;
					entry.buffer = std::get<wgpu::Buffer>(e.resource);
					entry.offset = e.offset;
					entry.size = e.size;
					entries.push_back(entry);
					break;
				}
				case 1: {
					wgpu::BindGroupEntry entry = {};
					entry.setDefault();
					entry.binding = e.binding;
					entry.textureView = std::get<wgpu::TextureView>(e.resource);
					entry.offset = e.offset;
					entry.size = e.size;
					entries.push_back(entry);
					break;
				}
				default:
					break;
				}
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

	void resizeRenderTargets(glm::ivec2 viewportSize);

	// render targets
	wgpu::Texture idBufferTexture;
	wgpu::TextureView idBufferTextureView;
	wgpu::Texture depthBufferTexture;
	wgpu::TextureView depthBufferTextureView;
	wgpu::Texture colorBufferTexture;
	wgpu::TextureView colorBufferTextureView;
	wgpu::Texture normalBufferTexture;
	wgpu::TextureView normalBufferTextureView;
	wgpu::Texture lightingBufferTexture;
	wgpu::TextureView lightingBufferTextureView;

	const std::shared_ptr<Mesh> &getFullscreenQuad() const { return fullscreenQuad; }
	const std::shared_ptr<Shader> &getLightingPassShader() const { return lightingPassShader; }

  private:
	RenderObjectCache renderObjectCache;

	Window &window;
	RendererResourceManager rendererResourcesManager;
	AssetManager &assetManager;

	Device device;

	std::shared_ptr<Mesh> fullscreenQuad = nullptr;
	std::shared_ptr<Shader> lightingPassShader = nullptr;
};

} // namespace CitronGraphics
