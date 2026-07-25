#pragma once

#include "serialization.hpp"
#include <assets.hpp>
#include <core.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class Shader : public Asset {
  public:
	Shader(const UUID uuid) : Asset(uuid) {}
	~Shader() = default;

	const std::string &getSourcePath() const { return sourcePath; }

	virtual void loadFromFile(const std::string &filepath) override;

	bool operator==(const Shader &other) const {
		return sourcePath == other.sourcePath;
	}

	wgpu::ShaderModule &getShaderModule() { return shaderModule; }

  private:
	const std::string sourcePath;
	wgpu::ShaderModule shaderModule;
};
} // namespace CitronGraphics
