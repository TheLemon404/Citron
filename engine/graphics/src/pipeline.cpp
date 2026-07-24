#include "pipeline.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Pipeline::Pipeline(wgpu::Device &device, wgpu::TextureView &colorTargetView,
				   Shader &vertexShader, Shader &fragmentShader)
	: device(device), colorTargetView(colorTargetView),
	  vertexShader(vertexShader), fragmentShader(fragmentShader) {

	wgpu::RenderPipelineDescriptor pipelineDesc;
	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;
	pipelineDesc.vertex.module = vertexShader.getShaderModule();
	pipelineDesc.vertex.entryPoint = wgpu::StringView("vs_main");
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

	wgpu::FragmentState fragmentState;
	fragmentState.module = fragmentShader.getShaderModule();
	fragmentState.entryPoint = wgpu::StringView("fs_main");
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;
	wgpu::BlendState blendState;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;
	wgpu::ColorTargetState colorTarget;
	colorTarget.format = colorTarget.format;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All;
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = nullptr;
	pipelineDesc.multisample.count = 2;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	pipeline = device.createRenderPipeline(pipelineDesc);
}
