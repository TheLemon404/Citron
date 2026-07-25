#pragma once

#include "device.hpp"
#include <assets.hpp>
#include <core.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class Shader : public Asset<Shader, AssetType::SHADER> {
  public:
	Shader(const UUID uuid, wgpu::ShaderModule shaderModule) : Asset<Shader, AssetType::SHADER>(uuid), shaderModule(shaderModule) {}
	~Shader() = default;

	bool operator==(const Shader &other) const {
		return shaderModule == other.shaderModule;
	}

	wgpu::ShaderModule &getShaderModule() { return shaderModule; }
	void setShaderModule(wgpu::ShaderModule &shaderModule) { this->shaderModule = shaderModule; }

  private:
	wgpu::ShaderModule shaderModule;
};

class ShaderImporter {
  public:
	ShaderImporter(AssetManager &assetManager, Device &device) : device(device) {
		assetManager.registerLoadFunction(AssetType::SHADER, std::bind(&ShaderImporter::loadShader, this, std::placeholders::_1, std::placeholders::_2));
	}

	std::shared_ptr<Shader> loadShader(UUID uuid, const std::string &filepath);

  private:
	Device &device;
};

} // namespace CitronGraphics
