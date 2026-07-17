#pragma once

#define WEBGPU_BACKEND DAWN

#include <webgpu/webgpu.hpp>
#include <window.hpp>

using namespace CitronCore;

namespace CitronGraphics {

class Device {
  public:
	Device(Window &window);
	~Device() = default;

	void aquirePlatformResources();
	void releasePlatformResources();

	bool constructRenderPass();
	void submitCommandBuffers();

	const void *getWGPUDevice() const { return device; }
	const void *getWGPUSurface() const { return surface; }
	const wgpu::TextureFormat getWGPUPreferredSurfaceFormat() const {
		return preferredSurfaceFormat;
	}
	const wgpu::RenderPassEncoder &getWGPURenderPassEncoder() const {
		return currentRenderPassEncoder;
	}

	const int getLastSurfaceWidth() const;
	const int getLastSurfaceHeight() const;

	void resizeSurface(int width, int height);

  private:
	int m_lastSurfaceWidth = 0;
	int m_lastSurfaceHeight = 0;
	wgpu::TextureFormat preferredSurfaceFormat;
	Window &window;
	std::vector<wgpu::CommandBuffer> commandBuffers;
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	wgpu::SurfaceTexture currentSurfaceTexture;
	wgpu::RenderPassEncoder currentRenderPassEncoder;
	wgpu::CommandEncoder currentCommandEncoder;
	wgpu::TextureView currentView;
};
} // namespace CitronGraphics
