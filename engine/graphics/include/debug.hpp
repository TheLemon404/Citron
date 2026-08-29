#pragma once

#include "graphics_exports.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace CitronGraphics {

struct CITRON_GRAPHICS_API DebugLine {
	glm::vec3 start;
	glm::vec3 end;
	glm::vec3 color = glm::vec3(1.0f);
};

} // namespace CitronGraphics
