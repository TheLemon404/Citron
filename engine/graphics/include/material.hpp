#pragma once

#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

class Material : public Asset<Material, AssetType::MATERIAL> {
  public:
	Material(const UUID uuid) : Asset<Material, AssetType::MATERIAL>(uuid) {}

	virtual void loadFromFile(const std::string &filepath) override;
};

} // namespace CitronGraphics
