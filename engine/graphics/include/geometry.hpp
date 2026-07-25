#pragma once

#include "assets.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace CitronAssets;

namespace CitronGraphics {
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};

class Geometry : public Asset {
  public:
	Geometry(const UUID uuid) : Asset(uuid) {}

	virtual void loadFromFile(const std::string &filepath) override;

  private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};
} // namespace CitronGraphics
