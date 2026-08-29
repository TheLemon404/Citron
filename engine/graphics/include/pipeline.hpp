#pragma once

#include "graphics_exports.hpp"
#include "shader.hpp"
#include <webgpu/webgpu.hpp>

namespace CitronGraphics {
enum class PipelineCullMode {
	None,
	Front,
	Back,
};

class CITRON_GRAPHICS_API Pipeline {
  public:
	Pipeline(wgpu::Device &device, const std::vector<wgpu::TextureFormat> colorAttachmentFormats, bool hasDepthStencilAttachment, std::shared_ptr<Shader> shader, PipelineCullMode cullMode = PipelineCullMode::Back, wgpu::PrimitiveTopology topology = wgpu::PrimitiveTopology::TriangleList);
	~Pipeline();
	void draw(wgpu::CommandEncoder &encoder);

	wgpu::RenderPipeline &getPipeline() { return pipeline; }

  private:
	std::vector<wgpu::ColorTargetState> colorTargets;
	wgpu::RenderPipeline pipeline;
	wgpu::Device &device;
};
} // namespace CitronGraphics
