#pragma once

#include "serialization.hpp"
#include <assets.hpp>
#include <core.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class Shader : public Asset {
  public:
	Shader(const UUID uuid, const std::string &sourcePath)
		: Asset(uuid), sourcePath(sourcePath) {}
	~Shader() = default;

	const std::string &getSourcePath() const { return sourcePath; }

	bool operator==(const Shader &other) const {
		return sourcePath == other.sourcePath;
	}

	wgpu::ShaderModule &getShaderModule() { return shaderModule; }

  private:
	const std::string sourcePath;
	wgpu::ShaderModule shaderModule;
};
} // namespace CitronGraphics
