#include "registry.hpp"
#include "assets.hpp"
#include "component.hpp"
#include "glm/ext/vector_float2.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "serialization.hpp"
#include "shader.hpp"
#include "test_system.hpp"
#include "uuid.hpp"
#include "view.hpp"
#include <lang.hpp>
#include <unordered_map>

using namespace CitronECS;

std::unordered_map<uint32_t, ComponentMetadata> ECSRegistry::m_componentRegistry;
std::unordered_map<uint32_t, PropertyGuiDrawer> ECSRegistry::m_propertyGuiDrawers;
std::unordered_map<uint32_t, SystemMetadata> ECSRegistry::m_systemRegistry;

std::unordered_map<uint32_t, std::function<void(StreamWriter &, void *)>> Member::serializationMethods;
std::unordered_map<uint32_t, std::function<void(StreamReader &, void *)>> Member::deserializationMethods;

void ECSRegistry::registerDefaultComponents() {
	Member::serializationMethods[typeid(int).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(int));
	};
	Member::serializationMethods[typeid(UUID).hash_code()] = [](StreamWriter &writer, void *data) {
		uint32_t rawID = static_cast<uint32_t *>(data)[0];
		writer.writeData(&rawID, sizeof(uint32_t));
	};
	Member::serializationMethods[typeid(uint64_t).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(uint64_t));
	};
	Member::serializationMethods[typeid(uint32_t).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(uint32_t));
	};
	Member::serializationMethods[typeid(float).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(float));
	};
	Member::serializationMethods[typeid(double).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(double));
	};
	Member::serializationMethods[typeid(std::string).hash_code()] = [](StreamWriter &writer, void *data) {
		std::string *str = (std::string *)data;
		writer.writeString(*str);
	};
	Member::serializationMethods[typeid(glm::vec2).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec2));
	};
	Member::serializationMethods[typeid(glm::vec3).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec3));
	};
	Member::serializationMethods[typeid(glm::vec4).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec4));
	};
	Member::serializationMethods[typeid(glm::mat4).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::mat4));
	};
	Member::serializationMethods[typeid(glm::quat).hash_code()] = [](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::quat));
	};
	Member::serializationMethods[typeid(PerspectiveView).hash_code()] = [](StreamWriter &writer, void *data) {
		PerspectiveView *perspectiveView = (PerspectiveView *)data;
		writer.writeData(&perspectiveView->aspect, sizeof(perspectiveView->aspect));
		writer.writeData(&perspectiveView->fov, sizeof(perspectiveView->fov));
		writer.writeData(&perspectiveView->near, sizeof(perspectiveView->near));
		writer.writeData(&perspectiveView->far, sizeof(perspectiveView->far));
	};
	registerCollectionSerialization<int>();
	registerCollectionSerialization<uint32_t>();
	registerCollectionSerialization<float>();
	registerCollectionSerialization<glm::vec3>();
	registerCollectionSerialization<UUID>();

	registerAssetReferenceSerialization<Mesh>();
	registerAssetReferenceSerialization<Material>();
	registerAssetReferenceSerialization<Shader>();

	Member::deserializationMethods[typeid(int).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(int));
	};
	Member::deserializationMethods[typeid(UUID).hash_code()] = [](StreamReader &reader, void *data) {
		uint32_t rawID;
		reader.readData(&rawID, sizeof(uint32_t));
		static_cast<uint32_t *>(data)[0] = rawID;
	};
	Member::deserializationMethods[typeid(uint64_t).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(uint64_t));
	};
	Member::deserializationMethods[typeid(uint32_t).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(uint32_t));
	};
	Member::deserializationMethods[typeid(float).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(float));
	};
	Member::deserializationMethods[typeid(double).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(double));
	};
	Member::deserializationMethods[typeid(std::string).hash_code()] = [](StreamReader &reader, void *data) {
		std::string *str = (std::string *)data;
		reader.readString(*str);
	};
	Member::deserializationMethods[typeid(glm::vec2).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec2));
	};
	Member::deserializationMethods[typeid(glm::vec3).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec3));
	};
	Member::deserializationMethods[typeid(glm::vec4).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec4));
	};
	Member::deserializationMethods[typeid(glm::mat4).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::mat4));
	};
	Member::deserializationMethods[typeid(glm::quat).hash_code()] = [](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::quat));
	};
	Member::deserializationMethods[typeid(PerspectiveView).hash_code()] = [](StreamReader &reader, void *data) {
		PerspectiveView *perspectiveView = (PerspectiveView *)data;
		reader.readData(&perspectiveView->aspect, sizeof(perspectiveView->aspect));
		reader.readData(&perspectiveView->fov, sizeof(perspectiveView->fov));
		reader.readData(&perspectiveView->near, sizeof(perspectiveView->near));
		reader.readData(&perspectiveView->far, sizeof(perspectiveView->far));
	};
	registerCollectionDeserialization<int>();
	registerCollectionDeserialization<uint32_t>();
	registerCollectionDeserialization<float>();
	registerCollectionDeserialization<glm::vec3>();
	registerCollectionDeserialization<UUID>();

	registerAssetReferenceDeserialization<Mesh>();
	registerAssetReferenceDeserialization<Material>();
	registerAssetReferenceDeserialization<Shader>();

	registerComponent<EntityBaseComponent>("Entity Base Component");
	registerComponentMember<EntityBaseComponent, uint32_t>("uuid", offsetof(EntityBaseComponent, uuid));
	registerComponentMember<EntityBaseComponent, std::string>("name", offsetof(EntityBaseComponent, name));
	registerComponentMember<EntityBaseComponent, uint32_t>("parentId", offsetof(EntityBaseComponent, parentId), true);
	registerComponentMember<EntityBaseComponent, std::vector<uint32_t>>("children", offsetof(EntityBaseComponent, children), true);
	registerComponent<TransformComponent>("Transform Component");
	registerComponentMember<TransformComponent, glm::vec3>("position", offsetof(TransformComponent, position));
	registerComponentMember<TransformComponent, glm::quat>("rotation", offsetof(TransformComponent, rotation));
	registerComponentMember<TransformComponent, glm::vec3>("scale", offsetof(TransformComponent, scale));
	registerComponent<MeshComponent>("Mesh Component");
	registerComponentMember<MeshComponent, AssetReference<Mesh>>("mesh", offsetof(MeshComponent, meshAsset));
	registerComponentMember<MeshComponent, AssetReference<Material>>("material", offsetof(MeshComponent, materialAsset));
	registerComponent<PerspectiveCameraComponent>("Perspective Camera Component");
	registerComponentMember<PerspectiveCameraComponent, PerspectiveView>("view", offsetof(PerspectiveCameraComponent, view));
}

void ECSRegistry::registerDefaultSystems() {
	registerSystem<TestSystem>("Test System");
	registerSystemMember<TestSystem, float>("test Member", offset_of<&TestSystem::testMember, TestSystem>());
}
