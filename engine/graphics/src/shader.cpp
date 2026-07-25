#include "shader.hpp"
#include "assets.hpp"
#include "logger.hpp"
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

std::shared_ptr<AssetBase> ShaderImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Loading shader: {}", metadata.assetPath.string());

	std::string source = CitronIO::IO::readFile(metadata.assetPath.string());

	wgpu::ShaderSourceWGSL shaderSource;
	shaderSource.chain.next = nullptr;
	shaderSource.chain.sType = wgpu::SType::ShaderSourceWGSL;
	shaderSource.code = WGPUStringView(source.data(), source.size());

	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct *>(&shaderSource);

	wgpu::ShaderModule shaderModule = device.getWGPUDevice().createShaderModule(shaderDesc);
	CITRON_CORE_INFO("Successfully compiled shader: {}", metadata.assetPath.string());
	return std::make_shared<Shader>(metadata.uuid, shaderModule);
}
