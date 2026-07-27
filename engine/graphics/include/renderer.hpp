#pragma once

#include "citron_exports.hpp"

#include "assets.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <cstddef>
#include <functional>
#include <layer.hpp>
#include <material.hpp>
#include <memory>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

namespace CitronGraphics {

class CITRON_GRAPHICS_API Frame;

struct CITRON_GRAPHICS_API RenderObject {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Shader> shader;
};

class CITRON_GRAPHICS_API Renderer;

class CITRON_GRAPHICS_API RenderPass {
  public:
	RenderPass(Renderer &renderer, Device &device, wgpu::Texture &targetTexture,
			   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame);
	~RenderPass();
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass(RenderPass &&) = default;
	RenderPass &operator=(RenderPass &&) = default;

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	void setPipeline(std::shared_ptr<Pipeline> pipeline);
	void setMesh(std::shared_ptr<Mesh> geometry);
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
	Device &device;
};

class CITRON_GRAPHICS_API Frame {
  public:
	Frame(Renderer &renderer, Device &device, wgpu::CommandEncoder encoder,
		  wgpu::SurfaceTexture &surfaceTexture)
		: renderer(renderer), device(device), encoder(encoder), surfaceTexture(surfaceTexture) {}

	RenderPass beginRenderPass(wgpu::Texture &tartetTexture);

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	wgpu::CommandEncoder &getEncoder() { return encoder; }
	wgpu::SurfaceTexture &getSurfaceTexture() { return surfaceTexture; }

  private:
	Renderer &renderer;
	wgpu::SurfaceTexture &surfaceTexture;
	Device &device;
	wgpu::CommandEncoder encoder;
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

	Device &getDevice() { return device; }

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

	std::shared_ptr<Pipeline> getPipeline(std::shared_ptr<Shader> shader, wgpu::TextureFormat format) {
		if (!pipelineCache.contains(shader)) {
			auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), shader, format);
			pipelineCache[shader] = pipeline;
		}
		return pipelineCache[shader];
	}

  private:
	Device device;
	AssetManager &assetManager;
	std::map<std::shared_ptr<Shader>, std::shared_ptr<Pipeline>> pipelineCache;
};
} // namespace CitronGraphics
