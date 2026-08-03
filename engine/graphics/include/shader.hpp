#pragma once

#include "device.hpp"
#include <assets.hpp>
#include <core.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class CITRON_GRAPHICS_API Shader : public Asset<Shader, AssetType::SHADER> {
  public:
	Shader(const UUID uuid, Device &device, std::string &source);
	~Shader();

	bool operator==(const Shader &other) const {
		return shaderModule == other.shaderModule;
	}

	wgpu::ShaderModule &getShaderModule() { return shaderModule; }
	void setShaderModule(wgpu::ShaderModule &shaderModule) { this->shaderModule = shaderModule; }

	template <typename T>
	static size_t dynamicSizeof(size_t alignment) {
		if (alignment == 0)
			return sizeof(T);
		return ((sizeof(T) + alignment - 1) / alignment) * alignment;
	}

	static size_t dynamicSizeof(std::size_t size, size_t alignment) {
		if (alignment == 0)
			return size;
		return ((size + alignment - 1) / alignment) * alignment;
	}

	template <typename T>
	static size_t paddedSizeof() {
		return ((sizeof(T) + 15) / 16) * 16;
	}

	static size_t paddedSizeof(std::size_t size) {
		return ((size + 15) / 16) * 16;
	}

	wgpu::PipelineLayout &getPipelineLayout() { return pipelineLayout; }
	wgpu::BindGroupLayout &getBindGroupLayout(const uint16_t index) { return bindGroupLayouts[index]; }

  private:
	wgpu::PipelineLayout pipelineLayout;
	std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
	wgpu::ShaderModule shaderModule;
};

class CITRON_GRAPHICS_API ShaderImporter : public AssetImporter {
  public:
	ShaderImporter(Device &device) : AssetImporter({".wgsl"}), device(device) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};

} // namespace CitronGraphics
