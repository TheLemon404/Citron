#include "shader.hpp"
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

std::shared_ptr<Shader> ShaderImporter::loadShader(UUID uuid, const std::string &filepath) {

	wgpu::ShaderSourceWGSL shaderSource;
	shaderSource.code = WGPUStringView(CitronIO::IO::readFile(filepath).c_str());

	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = (WGPUChainedStruct *)&shaderSource;

	return std::make_shared<Shader>(uuid, device.getWGPUDevice().createShaderModule(shaderDesc));
}
