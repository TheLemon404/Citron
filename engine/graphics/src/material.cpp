#include "material.hpp"
#include <yaml-cpp/yaml.h>

using namespace CitronGraphics;

std::shared_ptr<Material> MaterialImporter::loadMaterial(UUID uuid, const std::string &filepath) {
	std::shared_ptr<Material> material = std::make_shared<Material>(uuid);
	YAML::Node materialNode = YAML::LoadFile(filepath);
	material->shader.uuid = materialNode["shader"].as<uint64_t>();
	return material;
}
