#include "pipeline.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Pipeline::Pipeline(wgpu::Device &device,
				   std::shared_ptr<Shader> shader, wgpu::TextureFormat format)
	: device(device) {

	// render pipeline
	wgpu::VertexAttribute positionAttribute;
	positionAttribute.format = wgpu::VertexFormat::Float32x3;
	positionAttribute.offset = offsetof(Vertex, position);
	positionAttribute.shaderLocation = 0;

	wgpu::VertexAttribute normalAttribute;
	normalAttribute.format = wgpu::VertexFormat::Float32x3;
	normalAttribute.offset = offsetof(Vertex, normal);
	normalAttribute.shaderLocation = 1;

	wgpu::VertexAttribute colorAttribute;
	colorAttribute.format = wgpu::VertexFormat::Float32x3;
	colorAttribute.offset = offsetof(Vertex, color);
	colorAttribute.shaderLocation = 2;

	wgpu::VertexAttribute uvAttribute;
	uvAttribute.format = wgpu::VertexFormat::Float32x2;
	uvAttribute.offset = offsetof(Vertex, uv);
	uvAttribute.shaderLocation = 3;

	std::array<wgpu::VertexAttribute, 4> attributes = {positionAttribute, normalAttribute, colorAttribute, uvAttribute};

	wgpu::VertexBufferLayout vertexBufferLayout = {};
	vertexBufferLayout.attributes = attributes.data();
	vertexBufferLayout.attributeCount = static_cast<uint32_t>(attributes.size());
	vertexBufferLayout.arrayStride = sizeof(Vertex);
	vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

	wgpu::RenderPipelineDescriptor pipelineDesc = {};
	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;
	pipelineDesc.vertex.module = shader->getShaderModule();
	pipelineDesc.vertex.entryPoint = wgpu::StringView("vs_main");
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

	wgpu::FragmentState fragmentState = {};
	fragmentState.module = shader->getShaderModule();
	fragmentState.entryPoint = wgpu::StringView("fs_main");
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;
	wgpu::BlendState blendState;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;
	wgpu::ColorTargetState colorTarget;
	colorTarget.format = format;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All;
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = nullptr;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	pipelineDesc.layout = shader->getPipelineLayout();

	pipeline = device.createRenderPipeline(pipelineDesc);
}

Pipeline::~Pipeline() {
	pipeline.release();
}
