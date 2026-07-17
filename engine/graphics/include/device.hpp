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

	void constructRenderPass();
	void submitCommandBuffers();

	const void *getWGPUDevice() const { return device; }
	const void *getWGPUSurface() const { return surface; }
	const wgpu::TextureFormat getWGPUPreferredSurfaceFormat() const {
		return preferredSurfaceFormat;
	}
	const wgpu::RenderPassEncoder &getWGPURenderPassEncoder() const {
		return currentRenderPassEncoder;
	}

	void resizeSurface(int width, int height);

  private:
	bool isResizing = false;
	wgpu::TextureFormat preferredSurfaceFormat;
	Window &window;
	std::vector<wgpu::CommandBuffer> commandBuffers;
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	wgpu::RenderPassEncoder currentRenderPassEncoder;
	wgpu::CommandEncoder currentCommandEncoder;
	wgpu::TextureView currentView;
};
} // namespace CitronGraphics
