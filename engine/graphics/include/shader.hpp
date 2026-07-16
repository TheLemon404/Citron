#pragma once

#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class Shader : public ILoadable<Shader> {
  public:
	Shader() = default;
	~Shader() = default;

  private:
	void load(const std::string &assetSource) override;
};
} // namespace CitronGraphics
