#pragma once

#define WEBGPU_BACKEND DAWN

#include <webgpu/webgpu.hpp>
#include <window.hpp>

using namespace CitronCore;

namespace CitronGraphics {
class Context {
public:
  Context(Window &window) {};
  ~Context() = default;

  void aquirePlatformResources();

private:
  wgpu::Instance instance;
  wgpu::Adapter adapter;
  wgpu::Device device;
};
} // namespace CitronGraphics
