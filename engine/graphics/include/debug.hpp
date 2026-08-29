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

class CITRON_GRAPHICS_API DebugUtils {
  public:
	static void initialize(std::vector<DebugLine> *debugLinesList);
	static void addDebugLines(const std::vector<DebugLine> &lines, glm::vec3 color = glm::vec3(1.0f));
	static void addDebugLine(glm::vec3 start, glm::vec3 end, glm::vec3 color = glm::vec3(1.0f));
	static void addDebugCube(glm::vec3 min, glm::vec3 max, glm::vec3 color = glm::vec3(1.0f));

  private:
	static std::vector<DebugLine> *s_debugLinesList;
};

} // namespace CitronGraphics
