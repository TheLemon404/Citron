#pragma once

#include "citron_exports.hpp"
#include <glm/glm.hpp>

namespace CitronGraphics {
class CITRON_GRAPHICS_API View {
  public:
	glm::mat4 getViewMatrix();
	virtual glm::mat4 getProjectionMatrix() = 0;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, -10.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
};

class CITRON_GRAPHICS_API PerspectiveView : public View {
  public:
	float fov = 100.0f;
	float near = 0.01f;
	float far = 20000.0f;
	float aspect = 1.2f;
	glm::mat4 getProjectionMatrix() override;
};

} // namespace CitronGraphics
