#include "shader.hpp"
#include "assets.hpp"
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

std::shared_ptr<AssetBase> ShaderImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Loading shader: {}", metadata.assetPath.string());

	wgpu::ShaderSourceWGSL shaderSource;
	shaderSource.code = WGPUStringView(CitronIO::IO::readFile(metadata.assetPath.string()).c_str());

	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = (WGPUChainedStruct *)&shaderSource;

	return std::make_shared<Shader>(metadata.uuid, device.getWGPUDevice().createShaderModule(shaderDesc));
}
