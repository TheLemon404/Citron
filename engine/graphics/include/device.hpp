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

	void submitCommandBuffers();

  private:
	Window &window;
	std::vector<wgpu::CommandBuffer> commandBuffers;
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
};
} // namespace CitronGraphics
