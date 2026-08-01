#pragma once

#include "citron_exports.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "uuid.hpp"
#include <assets.hpp>
#include <cereal/types/vector.hpp>
#include <cstdint>
#include <entt/entt.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <layer.hpp>
#include <material.hpp>
#include <mesh.hpp>
#include <serialization.hpp>

using namespace CitronCore;
using namespace CitronAssets;
using namespace CitronGraphics;

namespace glm {

template <class Archive>
void serialize(Archive &archive, glm::vec2 &vec) {
	archive(vec.x, vec.y);
}

template <class Archive>
void serialize(Archive &archive, glm::vec3 &vec) {
	archive(vec.x, vec.y, vec.z);
}

template <class Archive>
void serialize(Archive &archive, glm::vec4 &vec) {
	archive(vec.x, vec.y, vec.z, vec.w);
}

template <class Archive>
void serialize(Archive &archive, glm::quat &quat) {
	archive(quat.x, quat.y, quat.z, quat.w);
}

template <class Archive>
void serialize(Archive &archive, glm::mat4 &mat) {
	archive(mat[0], mat[1], mat[2], mat[3]);
}
} // namespace glm

namespace CitronECS {

struct CITRON_ECS_API EntityBaseComponent {
	uint64_t uuid;
	std::string name;

	uint64_t parentId = UUID(0);
	std::vector<uint64_t> children;

	template <class Archive>
	void serialize(Archive &archive) {
		archive(uuid, name, parentId, children);
	}
};

struct CITRON_ECS_API TransformComponent {
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::identity<glm::quat>();
	glm::vec3 scale = glm::vec3(1.0f);

	// DO NOT DISPLAY THIS MEMBER IN EDITOR
	glm::mat4 matrix = glm::identity<glm::mat4>();

	template <class Archive>
	void serialize(Archive &archive) {
		archive(position, rotation, scale, matrix);
	}
};

struct CITRON_ECS_API MeshComponent {
	AssetReference<Mesh> meshAsset;
	AssetReference<Material> materialAsset;

	template <class Archive>
	void serialize(Archive &archive) {
		archive(meshAsset, materialAsset);
	}
};

} // namespace CitronECS
