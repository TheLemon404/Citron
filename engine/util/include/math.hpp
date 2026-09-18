#pragma once

#include "util_exports.hpp"
#include <glm/glm.hpp>

class CITRON_UTIL_API MathUtils {
  public:
	static bool rayIntersectsAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const glm::vec3 &aabbMin, const glm::vec3 &aabbMax);
};
