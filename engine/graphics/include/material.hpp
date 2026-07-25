#pragma once

#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

class Material : public Asset {
  public:
	Material(const UUID uuid) : Asset(uuid) {}

	virtual void loadFromFile(const std::string &filepath) override;
};

} // namespace CitronGraphics
