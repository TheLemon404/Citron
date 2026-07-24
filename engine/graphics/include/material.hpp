#pragma once

#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

class Material : public Asset {
  public:
	virtual void loadFromFile(const std::string &filepath);
};

} // namespace CitronGraphics
