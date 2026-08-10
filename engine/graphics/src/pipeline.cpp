#include "pipeline.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Pipeline::Pipeline(wgpu::Device &device, const std::vector<wgpu::TextureFormat> colorAttachmentFormats, bool hasDepthStencilAttachment,
				   std::shared_ptr<Shader> shader, PipelineCullMode cullMode)
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
	switch (cullMode) {
	case PipelineCullMode::Back:
		pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;
		break;
	case PipelineCullMode::Front:
		pipelineDesc.primitive.cullMode = wgpu::CullMode::Front;
		break;
	case PipelineCullMode::None:
		pipelineDesc.primitive.cullMode = wgpu::CullMode::None;
		break;
	}

	wgpu::FragmentState fragmentState = {};
	fragmentState.module = shader->getShaderModule();
	fragmentState.entryPoint = wgpu::StringView("fs_main");
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;
	wgpu::BlendState blendState;
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

	std::vector<wgpu::BlendState> targetBlends(colorAttachmentFormats.size(), blendState);

	for (size_t i = 0; i < colorAttachmentFormats.size(); i++) {
		wgpu::ColorTargetState colorTarget;
		colorTarget.format = colorAttachmentFormats[i];
		colorTarget.blend = &targetBlends[i];
		colorTarget.writeMask = wgpu::ColorWriteMask::All;
		colorTargets.push_back(colorTarget);
	}

	fragmentState.targetCount = colorTargets.size();
	fragmentState.targets = colorTargets.data();

	wgpu::DepthStencilState depthStencilState = {};
	if (hasDepthStencilAttachment) {
		depthStencilState.format = wgpu::TextureFormat::Depth32Float;
		depthStencilState.depthWriteEnabled = wgpu::OptionalBool::True;
		depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	}

	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = hasDepthStencilAttachment ? &depthStencilState : nullptr;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	pipelineDesc.layout = shader->getPipelineLayout();

	pipeline = device.createRenderPipeline(pipelineDesc);
}

Pipeline::~Pipeline() {
	pipeline.release();
}
