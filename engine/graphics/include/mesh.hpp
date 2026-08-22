#pragma once
#include "graphics_exports.hpp"

#include "assets.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace CitronAssets;

namespace CitronGraphics {
struct CITRON_GRAPHICS_API Vertex {
	Vertex(float x, float y, float z) : position(x, y, z) {}
	glm::vec3 position;
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(1.0f);
	glm::vec2 uv = glm::vec2(0.0f);
};

class CITRON_GRAPHICS_API Mesh : public Asset<Mesh, AssetType::MESH> {
  public:
	Mesh(const UUID uuid, Device &device) : Asset<Mesh, AssetType::MESH>(uuid), device(device) {}
	Mesh(const UUID uuid, std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 worldSpaceBoundsMin, glm::vec3 worldSpaceBoundsMax, Device &device);

	const GPUBuffer &getVertexBuffer() const { return vertexBuffer; }
	const GPUBuffer &getIndexBuffer() const { return indexBuffer; }

	const glm::vec3 &getBoundsMin() const { return boundsMin; }
	const glm::vec3 &getBoundsMax() const { return boundsMax; }

	static std::shared_ptr<Mesh> createFullscreenQuad(Device &device, AssetManager &assetManager);
	static std::shared_ptr<Mesh> createPlane(Device &device, AssetManager &assetManager);

  private:
	glm::vec3 boundsMin = glm::vec3(0.0f);
	glm::vec3 boundsMax = glm::vec3(0.0f);
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Device &device;

	GPUBuffer vertexBuffer;
	GPUBuffer indexBuffer;
};

class CITRON_GRAPHICS_API MeshImporter : public AssetImporter {
  public:
	MeshImporter(Device &device) : AssetImporter({".fbx", ".glb", ".gltf"}), device(device) {};

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};
} // namespace CitronGraphics
