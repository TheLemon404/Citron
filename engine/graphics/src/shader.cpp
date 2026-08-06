#include "shader.hpp"
#include "assets.hpp"
#include "logger.hpp"
#include "material.hpp"
#include "resources.hpp"
#include <libminiray.h>
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>
#include <nlohmann/json.hpp>

using namespace nlohmann;

using namespace CitronGraphics;

Shader::Shader(const UUID uuid, Device &device, std::string &source) : Asset<Shader, AssetType::SHADER>(uuid) {
	// bind group layout & pipeline layout
	char *reflectionData;
	int reflectionDataLength;
	if (miniray_reflect(source.data(), source.size(), &reflectionData, &reflectionDataLength)) {
		CITRON_CORE_ERROR("Failed to get miniray reflection data for shader: {}", (uint32_t)uuid);
		return;
	}

	json reflectionJson = json::parse(reflectionData);
	for (const auto &bindingEntry : reflectionJson["bindings"]) {
		const size_t group = bindingEntry["group"];
		const size_t binding = bindingEntry["binding"];
		const std::string name = bindingEntry["name"];
		const std::string type = bindingEntry["type"];

		wgpu::BindGroupLayoutEntry bindingLayout = {};
		bindingLayout.setDefault();
		bindingLayout.binding = binding;
		bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
		bindingLayout.buffer.type = wgpu::BufferBindingType::BindingNotUsed;
		bindingLayout.texture.sampleType = wgpu::TextureSampleType::BindingNotUsed;
		bindingLayout.texture.viewDimension = wgpu::TextureViewDimension::Undefined;
		bindingLayout.sampler.type = wgpu::SamplerBindingType::BindingNotUsed;
		bindingLayout.storageTexture.access = wgpu::StorageTextureAccess::BindingNotUsed;

		if (type == "texture_2d<f32>") {
			bindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
			bindingLayout.texture.viewDimension = wgpu::TextureViewDimension::_2D;
		} else if (bindingEntry.contains("layout")) {
			const size_t layoutSize = bindingEntry["layout"]["size"];
			const size_t layoutAlignment = bindingEntry["layout"]["alignment"];
			bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
			bindingLayout.buffer.minBindingSize = Shader::dynamicSizeof(layoutSize, layoutAlignment);
		}

		groupEntries[group].push_back(bindingLayout);
	}

	for (auto &[group, entries] : groupEntries) {
		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {};
		bindGroupLayoutDesc.nextInChain = nullptr;
		bindGroupLayoutDesc.entryCount = entries.size();
		bindGroupLayoutDesc.entries = entries.data();
		bindGroupLayouts.push_back(device.getWGPUDevice().createBindGroupLayout(bindGroupLayoutDesc));
	}

	miniray_free(reflectionData);

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
