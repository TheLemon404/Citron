#pragma once

#include "device.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <layer.hpp>
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

class Renderer {
  public:
	Renderer(Window &window) : device(window) {}

	bool frameReady() { return device.prepareCurrentSurfaceTexture(); }
	Frame beginFrame();
	void endFrame(Frame &frame);

	void init();
	void end();
	void onEvent(Event &e);

	Device &getDevice() { return device; }

	std::function<void(wgpu::TextureView &, RenderPass &)> onGuiDrawCallback =
		nullptr;

  private:
	Device device;
};
} // namespace CitronGraphics
