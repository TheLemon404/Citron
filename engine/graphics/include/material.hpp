#pragma once

#include "shader.hpp"
#include <assets.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

class Material : public Asset<Material, AssetType::MATERIAL> {
  public:
	Material(const UUID uuid) : Asset<Material, AssetType::MATERIAL>(uuid) {}

	AssetReference<Shader> shader;
};

class MaterialImporter {
  public:
	MaterialImporter(AssetManager &assetManager) {
		assetManager.registerLoadFunction(AssetType::MATERIAL, std::bind(&MaterialImporter::loadMaterial, this, std::placeholders::_1, std::placeholders::_2));
	}

	std::shared_ptr<Material> loadMaterial(UUID uuid, const std::string &filepath);
};

} // namespace CitronGraphics
