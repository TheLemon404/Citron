#pragma once

#include "shader.hpp"
#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

class CITRON_GRAPHICS_API Material : public Asset<Material, AssetType::MATERIAL> {
  public:
	Material(const UUID uuid) : Asset<Material, AssetType::MATERIAL>(uuid) {}

	AssetReference<Shader> shader;
};

class CITRON_GRAPHICS_API MaterialImporter : public AssetImporter {
  public:
	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;
};

} // namespace CitronGraphics
