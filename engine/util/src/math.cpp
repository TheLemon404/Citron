#include "math.hpp"
#include "glm/common.hpp"
#include <algorithm>

bool MathUtils::rayIntersectsAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const glm::vec3 &aabbMin, const glm::vec3 &aabbMax) {
	glm::vec3 tmin = (aabbMin - rayOrigin) / rayDir;
	glm::vec3 tmax = (aabbMax - rayOrigin) / rayDir;

	float tnear = std::max(std::max(std::min(tmin.x, tmax.x), std::min(tmin.y, tmax.y)), std::min(tmin.z, tmax.z));
	float tfar = std::min(std::min(std::max(tmin.x, tmax.x), std::max(tmin.y, tmax.y)), std::max(tmin.z, tmax.z));

	return tnear <= tfar && tfar >= 0;
}
