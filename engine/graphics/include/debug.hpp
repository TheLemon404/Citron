#pragma once

#include "graphics_exports.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace CitronGraphics {

struct CITRON_GRAPHICS_API DebugLine {
	glm::vec3 start;
	glm::vec3 end;
};

class CITRON_GRAPHICS_API DebugShape {
  public:
	std::vector<DebugLine> lines;
	glm::vec4 color;
};

} // namespace CitronGraphics
