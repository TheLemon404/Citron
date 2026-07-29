#include "shader.hpp"
#include "assets.hpp"
#include "logger.hpp"
#include "material.hpp"
#include "renderer.hpp"
#include <array>
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Shader::Shader(const UUID uuid, Device &device, std::string &source) : Asset<Shader, AssetType::SHADER>(uuid) {
	// bind group layout & pipeline layout
	wgpu::BindGroupLayoutEntry frameUniformBindingLayout = {};
	frameUniformBindingLayout.setDefault();
	frameUniformBindingLayout.binding = 0;
	frameUniformBindingLayout.visibility = wgpu::ShaderStage::Vertex;
	frameUniformBindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
	frameUniformBindingLayout.buffer.minBindingSize = Shader::getShaderPaddedBindingSize<FrameUniforms>();
	// IMPORTANT: THIS IS A BUG IN WGPU. setDefault() sets types to Undefine, not BindingNotUsed, which causes mysterous runtime errors
	frameUniformBindingLayout.texture.sampleType = wgpu::TextureSampleType::BindingNotUsed;
	frameUniformBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::Undefined;
	frameUniformBindingLayout.sampler.type = wgpu::SamplerBindingType::BindingNotUsed;
	frameUniformBindingLayout.storageTexture.access = wgpu::StorageTextureAccess::BindingNotUsed;

	wgpu::BindGroupLayoutDescriptor frameBindGroupLayoutDesc = {};
	frameBindGroupLayoutDesc.nextInChain = nullptr;
	frameBindGroupLayoutDesc.entryCount = 1;
	frameBindGroupLayoutDesc.entries = &frameUniformBindingLayout;
	bindGroupLayouts.push_back(device.getWGPUDevice().createBindGroupLayout(frameBindGroupLayoutDesc));

	wgpu::BindGroupLayoutEntry materialUniformBindingLayout = {};
	materialUniformBindingLayout.setDefault();
	materialUniformBindingLayout.binding = 0;
	materialUniformBindingLayout.visibility = wgpu::ShaderStage::Fragment;
	materialUniformBindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
	materialUniformBindingLayout.buffer.minBindingSize = Shader::getShaderPaddedBindingSize<MaterialUniforms>();
	materialUniformBindingLayout.texture.sampleType = wgpu::TextureSampleType::BindingNotUsed;
	materialUniformBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::Undefined;
	materialUniformBindingLayout.sampler.type = wgpu::SamplerBindingType::BindingNotUsed;
	materialUniformBindingLayout.storageTexture.access = wgpu::StorageTextureAccess::BindingNotUsed;

	wgpu::BindGroupLayoutDescriptor materialBindGroupLayoutDesc = {};
	materialBindGroupLayoutDesc.nextInChain = nullptr;
	materialBindGroupLayoutDesc.entryCount = 1;
	materialBindGroupLayoutDesc.entries = &materialUniformBindingLayout;
	bindGroupLayouts.push_back(device.getWGPUDevice().createBindGroupLayout(materialBindGroupLayoutDesc));

	wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
	pipelineLayoutDesc.setDefault();
	pipelineLayoutDesc.nextInChain = nullptr;
	pipelineLayoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
	pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *)bindGroupLayouts.data();
	pipelineLayout = device.getWGPUDevice().createPipelineLayout(pipelineLayoutDesc);

	// shader module creation
	wgpu::ShaderSourceWGSL shaderSource;
	shaderSource.chain.next = nullptr;
	shaderSource.chain.sType = wgpu::SType::ShaderSourceWGSL;
	shaderSource.code = WGPUStringView(source.data(), source.size());
	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct *>(&shaderSource);
	shaderModule = device.getWGPUDevice().createShaderModule(shaderDesc);
}

Shader::~Shader() {
	shaderModule.release();
	pipelineLayout.release();
	for (auto &layout : bindGroupLayouts) {
		layout.release();
	}
}

std::shared_ptr<AssetBase> ShaderImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Loading shader: {}", metadata.assetPath.string());

	std::string source = CitronIO::IO::readFile(metadata.assetPath.string());

	CITRON_CORE_INFO("Successfully compiled shader: {}", metadata.assetPath.string());
	return std::make_shared<Shader>(metadata.uuid, device, source);
}
