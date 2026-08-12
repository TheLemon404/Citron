#pragma once

#include "assets.hpp"
#include "buffer.hpp"
#include "citron_exports.hpp"

#include "debug.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "resources.hpp"
#include "shader.hpp"
#include "texture.hpp"
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
	Texture targetTexture;
	wgpu::TextureFormat textureFormat = wgpu::TextureFormat::BGRA8UnormSrgb;
	wgpu::Color clearValue = {0.247, 0.247, 0.247, 0.0f};
	wgpu::LoadOp loadOp = wgpu::LoadOp::Clear;
};

struct RenderPassDepthStencilAttachment {
	Texture targetTexture;
	float depthClearValue = 1.0f;
	float stencilClearValue = 0.0f;
};

struct RenderPassParams {
	std::vector<RenderPassColorAttachment> colorAttachments;
	bool containsDepthStencil = false;
	RenderPassDepthStencilAttachment depthStencilAttachment;
	wgpu::LoadOp depthLoadOp = wgpu::LoadOp::Clear;
	wgpu::LoadOp stencilLoadOp = wgpu::LoadOp::Clear;
};

class CITRON_GRAPHICS_API RenderPass {
  public:
	RenderPass(Renderer &renderer, RenderPassParams &params,
			   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame);
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass(RenderPass &&) = default;

	void drawFullscreenQuadPass(std::shared_ptr<Mesh> fullscreenQuad, std::shared_ptr<Shader> shader);
	void drawRenderData(std::vector<RenderObject> renderableReferenceData);
	void drawDebugGrid();
	void drawRebugRenderData(std::vector<RenderObject> renderableReferenceData, DrawUniforms frameUniforms);

	void setPipeline(std::shared_ptr<Pipeline> pipeline);
	void setMesh(std::shared_ptr<Mesh> geometry);
	void setBindGroup(int index, wgpu::BindGroup bindGroup);
	void draw(std::shared_ptr<Mesh> geometry);
	void end();

	wgpu::RenderPassEncoder &getRenderPassEncoder() {
		return renderPassEncoder;
	}

	wgpu::TextureView &getColorTargetView(int attachmentIndex) { return params.colorAttachments[attachmentIndex].targetTexture.getTextureView(); }

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
		  wgpu::SurfaceTexture &surfaceTexture) : renderer(renderer), encoder(encoder) {}

	RenderPass beginRenderPass(RenderPassParams &params);

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	wgpu::CommandEncoder &getEncoder() { return encoder; }

	void incrementRenderCount() { renderCount++; }
	const uint16_t getRenderCount() const { return renderCount; }

  private:
	uint16_t renderCount = 0;
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
	Renderer(Window &window, AssetManager &assetManager) : window(window), device(window), assetManager(assetManager), rendererResourcesManager(device, assetManager) {}

	bool frameReady();
	Frame beginFrame();
	void endFrame(Frame &frame);

	void render(Frame &frame, View &view, std::vector<RenderableReferenceData> renderableReferenceData, glm::ivec2 iviewportSize, Texture &outputTexture, bool drawDebug = false);

	void init();
	void end();
	void onEvent(Event &e);

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

	void createRenderTargetColorTexture(Texture &texture, uint32_t width, uint32_t height);

	Texture &getCurrentDeviceSurfaceTexture() {
		return deviceSurfaceTexture;
	}

	void prepareRenderTargetTextures(uint32_t width, uint32_t height);

	// render targets
	Texture idBufferTexture;
	Texture depthBufferTexture;
	Texture colorBufferTexture;
	Texture normalBufferTexture;
	Texture lightingBufferTexture;

	const std::shared_ptr<Mesh> &getFullscreenQuad() const { return fullscreenQuad; }
	const std::shared_ptr<Shader> &getLightingPassShader() const { return lightingPassShader; }

  private:
	std::stack<DebugShape> debugShapes;

	Texture deviceSurfaceTexture;

	RenderObjectCache renderObjectCache;

	Window &window;
	RendererResourceManager rendererResourcesManager;
	AssetManager &assetManager;

	Device device;

	std::shared_ptr<Mesh> fullscreenQuad = nullptr;
	std::shared_ptr<Shader> lightingPassShader = nullptr;
};

} // namespace CitronGraphics
