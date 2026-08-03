#pragma once

#include "shader.hpp"
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {
class CITRON_GRAPHICS_API Pipeline {
  public:
	Pipeline(wgpu::Device &device, const std::vector<wgpu::RenderPassColorAttachment> colorAttachments, bool hasDepthStencilAttachment, std::shared_ptr<Shader> shader, wgpu::TextureFormat format);
	~Pipeline();
	void draw(wgpu::CommandEncoder &encoder);

	wgpu::RenderPipeline &getPipeline() { return pipeline; }

  private:
	std::vector<wgpu::ColorTargetState> colorTargets;
	wgpu::RenderPipeline pipeline;
	wgpu::Device &device;
};
} // namespace CitronGraphics
