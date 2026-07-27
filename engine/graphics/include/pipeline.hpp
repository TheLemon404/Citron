#pragma once

#include "shader.hpp"
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {
class CITRON_GRAPHICS_API Pipeline {
  public:
	Pipeline(wgpu::Device &device, wgpu::Texture &targetTexture,
			 std::shared_ptr<Shader> shader);
	~Pipeline();
	void draw(wgpu::CommandEncoder &encoder);

	wgpu::RenderPipeline &getPipeline() { return pipeline; }

  private:
	wgpu::RenderPipeline pipeline;
	wgpu::Device &device;
	wgpu::Texture &targetTexture;
};
} // namespace CitronGraphics
