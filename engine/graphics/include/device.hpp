#pragma once

#include "glm/fwd.hpp"
#define WEBGPU_BACKEND DAWN

#include "buffer.hpp"
#include "citron_exports.hpp"
#include <webgpu/webgpu.hpp>
#include <window.hpp>

using namespace CitronCore;

namespace CitronGraphics {

class CITRON_GRAPHICS_API Device {
  public:
	Device(Window &window);
	~Device() = default;

	void aquirePlatformResources();
	void releasePlatformResources();

	bool prepareCurrentSurfaceTexture();
	void presentCurrentSurfaceTexture();

	wgpu::Device &getWGPUDevice() { return device; }
	const void *getWGPUDeviceRaw() { return &device; }
	wgpu::Surface &getWGPUSurface() { return surface; }
	const void *getWGPUSurfaceRaw() { return &surface; }
	wgpu::SurfaceTexture &getCurrentSurfaceTexture() {
		return currentSurfaceTexture;
	}
	const void *getCurrentSurfaceTextureRaw() { return &currentSurfaceTexture; }

	const wgpu::TextureFormat getWGPUPreferredSurfaceFormat() {
		return preferredSurfaceFormat;
	}
	const wgpu::RenderPassEncoder &createDisposableRenderPassEncoder();

	const int getLastSurfaceWidth();
	const int getLastSurfaceHeight();

	const wgpu::Queue &getQueue() { return queue; }
	const wgpu::Instance &getInstance() { return instance; }

	void resizeSurface(int width, int height);

	wgpu::Texture createRenderTargetDepthTexture(int width, int height);
	wgpu::Texture createRenderTargetColorTexture(int width, int height);
	wgpu::TextureView createTextureView(wgpu::Texture &texture);

  private:
	int m_lastSurfaceWidth = 0;
	int m_lastSurfaceHeight = 0;
	wgpu::TextureFormat preferredSurfaceFormat;
	Window &window;
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	wgpu::SurfaceTexture currentSurfaceTexture;
};
} // namespace CitronGraphics
