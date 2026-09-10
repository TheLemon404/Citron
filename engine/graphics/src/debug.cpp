#include "debug.hpp"
#include "clock.hpp"

using namespace CitronGraphics;

std::vector<DebugLine> *DebugUtils::s_debugLinesList = nullptr;

void DebugUtils::initialize(std::vector<DebugLine> *debugLinesList) {
	s_debugLinesList = debugLinesList;
}

void DebugUtils::addDebugLines(const std::vector<DebugLine> &lines, glm::vec3 color, float deleteTime) {
	for (const auto &line : lines) {
		addDebugLine(line.start, line.end, color, deleteTime);
	}
}

void DebugUtils::addDebugLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float deleteTime) {
	if (s_debugLinesList) {
		s_debugLinesList->push_back({start, end, color, CitronCore::Clock::getLastFrameTime() + deleteTime});
	}
}

void DebugUtils::addDebugCube(glm::vec3 min, glm::vec3 max, glm::vec3 color, float deleteTime) {
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

	addDebugLine(corners[0], corners[1], color, deleteTime);
	addDebugLine(corners[0], corners[2], color, deleteTime);
	addDebugLine(corners[0], corners[4], color, deleteTime);
	addDebugLine(corners[1], corners[3], color, deleteTime);
	addDebugLine(corners[1], corners[5], color, deleteTime);
	addDebugLine(corners[2], corners[3], color, deleteTime);
	addDebugLine(corners[2], corners[6], color, deleteTime);
	addDebugLine(corners[3], corners[7], color, deleteTime);
	addDebugLine(corners[4], corners[5], color, deleteTime);
	addDebugLine(corners[4], corners[6], color, deleteTime);
	addDebugLine(corners[5], corners[7], color, deleteTime);
	addDebugLine(corners[6], corners[7], color, deleteTime);
}
