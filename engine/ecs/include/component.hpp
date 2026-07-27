#pragma once

#include "citron_exports.hpp"
#include "uuid.hpp"
#include <assets.hpp>
#include <cstdint>
#include <entt/entt.hpp>
#include <geometry.hpp>
#include <layer.hpp>
#include <material.hpp>
#include <serialization.hpp>

using namespace CitronCore;
using namespace CitronAssets;
using namespace CitronGraphics;

namespace CitronECS {

struct CITRON_ECS_API EntityBaseComponent {
	uint64_t uuid;
	std::string name;

	uint64_t parentId = UUID(0);
	std::vector<uint64_t> children;
};

struct CITRON_ECS_API MeshComponent {
	AssetReference<Geometry> geometryAsset;
	AssetReference<Shader> materialAsset;
};

} // namespace CitronECS
