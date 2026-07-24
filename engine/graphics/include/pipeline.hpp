#pragma once

#include "shader.hpp"
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {
class Pipeline {
  public:
	Pipeline(wgpu::Device &device, wgpu::TextureView &colorTargetView,
			 Shader &vertexShader, Shader &fragmentShader);
	void draw(wgpu::CommandEncoder &encoder);

  private:
	wgpu::RenderPipeline pipeline;
	wgpu::Device &device;
	wgpu::TextureView &colorTargetView;
	Shader &vertexShader;
	Shader &fragmentShader;
};
} // namespace CitronGraphics
