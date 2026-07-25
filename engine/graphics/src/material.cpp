#include "material.hpp"
#include "logger.hpp"
#include <yaml-cpp/yaml.h>

using namespace CitronGraphics;

std::shared_ptr<AssetBase> MaterialImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Loading material: {}", metadata.assetPath.string());

	std::shared_ptr<Material> material = std::make_shared<Material>(metadata.uuid);
	YAML::Node materialNode = YAML::LoadFile(metadata.assetPath.string());
	material->shader.uuid = materialNode["shader"].as<uint64_t>();
	return material;
}
