#pragma once

#include "assets.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace CitronAssets;

namespace CitronGraphics {
struct Vertex {
	Vertex(float x, float y, float z) : position(x, y, z) {}
	glm::vec3 position;
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(1.0f);
	glm::vec2 uv = glm::vec2(0.0f);
};

class Geometry : public Asset<Geometry, AssetType::GEOMETRY> {
  public:
	Geometry(const UUID uuid, Device &device) : Asset<Geometry, AssetType::GEOMETRY>(uuid), device(device) {}
	Geometry(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Device &device);

	const GPUBuffer &getVertexBuffer() const { return vertexBuffer; }
	const GPUBuffer &getIndexBuffer() const { return indexBuffer; }

  private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Device &device;

	GPUBuffer vertexBuffer;
	GPUBuffer indexBuffer;
};

class GeometryImporter : public AssetImporter {
  public:
	GeometryImporter(Device &device) : AssetImporter(), device(device) {};

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};
} // namespace CitronGraphics
