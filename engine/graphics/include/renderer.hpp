#pragma once

#include "assets.hpp"
#include "device.hpp"
#include "geometry.hpp"
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

class Frame;

class RenderPass {
  public:
	RenderPass(wgpu::Device &device, wgpu::Texture &targetTexture,
			   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame);
	~RenderPass();
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass(RenderPass &&) = default;
	RenderPass &operator=(RenderPass &&) = default;

	void setPipeline(std::shared_ptr<Pipeline> pipeline) {
		renderPassEncoder.setPipeline(pipeline->getPipeline());
	}
	void setGeometry(std::shared_ptr<Geometry> geometry) {
		renderPassEncoder.setVertexBuffer(0, geometry->getVertexBuffer().buffer, 0, WGPU_WHOLE_SIZE);
		renderPassEncoder.setIndexBuffer(geometry->getIndexBuffer().buffer, wgpu::IndexFormat::Uint32, 0, WGPU_WHOLE_SIZE);
	}
	void draw(std::shared_ptr<Geometry> geometry) {
		renderPassEncoder.drawIndexed(geometry->getIndexBuffer().entryCount, 1, 0, 0, 0);
	}
	void end();

	wgpu::RenderPassEncoder &getRenderPassEncoder() {
		return renderPassEncoder;
	}

	wgpu::TextureView &getTargetView() { return targetView; }

	Frame &getParentFrame() { return parentFrame; }

  private:
	Frame &parentFrame;
	wgpu::Texture &targetTexture;
	wgpu::CommandEncoder &commandEncoder;
	wgpu::TextureView targetView;
	wgpu::RenderPassEncoder renderPassEncoder;
	wgpu::Device &device;
};

class Frame {
  public:
	Frame(wgpu::Device &device, wgpu::CommandEncoder encoder,
		  wgpu::SurfaceTexture &surfaceTexture)
		: device(device), encoder(encoder), surfaceTexture(surfaceTexture) {}

	RenderPass beginRenderPass(wgpu::Texture &tartetTexture);

	wgpu::CommandEncoder &getEncoder() { return encoder; }
	wgpu::SurfaceTexture &getSurfaceTexture() { return surfaceTexture; }

  private:
	wgpu::SurfaceTexture &surfaceTexture;
	wgpu::Device &device;
	wgpu::CommandEncoder encoder;
};

struct RenderObject {
	std::shared_ptr<Geometry> geometry;
	std::shared_ptr<Material> material;
};

class Renderer {
  public:
	Renderer(Window &window, AssetManager &assetManager) : device(window), assetManager(assetManager) {}

	bool frameReady() { return device.prepareCurrentSurfaceTexture(); }
	Frame beginFrame();
	void endFrame(Frame &frame);

	void drawRenderData(std::vector<RenderObject> &renderObjects);

	void init();
	void end();
	void onEvent(Event &e);

	Device &getDevice() { return device; }

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

	std::shared_ptr<Pipeline> getPipeline(std::shared_ptr<Shader> shader, wgpu::Texture &targetTexture) {
		if (!pipelineCache.contains(shader)) {
			auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), targetTexture, shader);
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
