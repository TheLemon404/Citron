#pragma once

#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class Shader : public ILoadable<Shader> {
  public:
	Shader(const std::string &sourcePath) : sourcePath(sourcePath) {}
	~Shader() = default;

	const std::string &getSourcePath() const { return sourcePath; }

	bool operator==(const Shader &other) const {
		return sourcePath == other.sourcePath;
	}

  private:
	const std::string sourcePath;
	void load(const std::string &assetSource) override;
};
} // namespace CitronGraphics
