#include "view.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

using namespace CitronGraphics;

glm::mat4 View::getViewMatrix() {
	return glm::lookAt(position, position + forward, -up);
}

glm::mat4 PerspectiveView::getProjectionMatrix() {
	return glm::perspective(fov, aspect, near, far);
}

bool PerspectiveView::isInsideBounds(glm::vec3 position) {
	glm::vec4 clipCoord = glm::vec4(position, 1.0f) * getProjectionMatrix() * getViewMatrix();
	clipCoord /= clipCoord.w;
	return clipCoord.x >= -1.0f && clipCoord.x <= 1.0f && clipCoord.y >= -1.0f && clipCoord.y <= 1.0f && clipCoord.z >= -1.0f && clipCoord.z <= 1.0f;
}
