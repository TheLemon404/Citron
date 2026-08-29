#include "debug.hpp"

using namespace CitronGraphics;

std::vector<DebugLine> *DebugUtils::s_debugLinesList = nullptr;

void DebugUtils::initialize(std::vector<DebugLine> *debugLinesList) {
	s_debugLinesList = debugLinesList;
}

void DebugUtils::addDebugLines(const std::vector<DebugLine> &lines, glm::vec3 color) {
	for (const auto &line : lines) {
		addDebugLine(line.start, line.end, color);
	}
}

void DebugUtils::addDebugLine(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
	if (s_debugLinesList) {
		s_debugLinesList->push_back({start, end, color});
	}
}

void DebugUtils::addDebugCube(glm::vec3 min, glm::vec3 max, glm::vec3 color) {
	glm::vec3 size = max - min;
	glm::vec3 center = min + size / 2.0f;
	glm::vec3 halfSize = size / 2.0f;
	glm::vec3 corners[8] = {
		center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
		center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
		center + glm::vec3(-halfSize.x, halfSize.y, -halfSize.z),
		center + glm::vec3(halfSize.x, halfSize.y, -halfSize.z),
		center + glm::vec3(-halfSize.x, -halfSize.y, halfSize.z),
		center + glm::vec3(halfSize.x, -halfSize.y, halfSize.z),
		center + glm::vec3(-halfSize.x, halfSize.y, halfSize.z),
		center + glm::vec3(halfSize.x, halfSize.y, halfSize.z),
	};

	addDebugLine(corners[0], corners[1], color);
	addDebugLine(corners[0], corners[2], color);
	addDebugLine(corners[0], corners[4], color);
	addDebugLine(corners[1], corners[3], color);
	addDebugLine(corners[1], corners[5], color);
	addDebugLine(corners[2], corners[3], color);
	addDebugLine(corners[2], corners[6], color);
	addDebugLine(corners[3], corners[7], color);
	addDebugLine(corners[4], corners[5], color);
	addDebugLine(corners[4], corners[6], color);
	addDebugLine(corners[5], corners[7], color);
	addDebugLine(corners[6], corners[7], color);
}
