#pragma once

#include "device.hpp"
#include <assets.hpp>
#include <core.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class CITRON_GRAPHICS_API Shader : public Asset<Shader, AssetType::SHADER> {
  public:
	Shader(const UUID uuid, wgpu::ShaderModule shaderModule) : Asset<Shader, AssetType::SHADER>(uuid), shaderModule(shaderModule) {}
	~Shader() {
		shaderModule.release();
	}

	bool operator==(const Shader &other) const {
		return shaderModule == other.shaderModule;
	}

	wgpu::ShaderModule &getShaderModule() { return shaderModule; }
	void setShaderModule(wgpu::ShaderModule &shaderModule) { this->shaderModule = shaderModule; }

	const uint16_t getAttributeCount() { return attributeCount; }

  private:
	const uint16_t attributeCount = 1;
	wgpu::ShaderModule shaderModule;
};

class CITRON_GRAPHICS_API ShaderImporter : public AssetImporter {
  public:
	ShaderImporter(Device &device) : device(device) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};

} // namespace CitronGraphics
