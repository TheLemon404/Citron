#pragma once

#include "uuid.hpp"
#include <assets.hpp>
#include <cstdint>
#include <entt/entt.hpp>
#include <layer.hpp>
#include <serialization.hpp>

using namespace CitronCore;
using namespace CitronAssets;

namespace CitronECS {

struct EntityBaseComponent {
	uint64_t uuid;
	std::string name;

	uint64_t parentId = UUID(0);
	std::vector<uint64_t> children;
};

} // namespace CitronECS
