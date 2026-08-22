#include "registry.hpp"
#include "assets.hpp"
#include "component.hpp"
#include "glm/ext/vector_float2.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "serialization.hpp"
#include "shader.hpp"
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
	registerSerializationMethod<int>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(int));
	});
	registerSerializationMethod<UUID>([](StreamWriter &writer, void *data) {
		uint32_t rawID = static_cast<uint32_t *>(data)[0];
		writer.writeData(&rawID, sizeof(uint32_t));
	});
	registerSerializationMethod<uint64_t>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(uint64_t));
	});
	registerSerializationMethod<uint32_t>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(uint32_t));
	});
	registerSerializationMethod<float>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(float));
	});
	registerSerializationMethod<double>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(double));
	});
	registerSerializationMethod<std::string>([](StreamWriter &writer, void *data) {
		std::string *str = (std::string *)data;
		writer.writeString(*str);
	});
	registerSerializationMethod<glm::vec2>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec2));
	});
	registerSerializationMethod<glm::vec3>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec3));
	});
	registerSerializationMethod<glm::vec4>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::vec4));
	});
	registerSerializationMethod<glm::mat4>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::mat4));
	});
	registerSerializationMethod<glm::quat>([](StreamWriter &writer, void *data) {
		writer.writeData(data, sizeof(glm::quat));
	});
	registerSerializationMethod<PerspectiveView>([](StreamWriter &writer, void *data) {
		PerspectiveView *perspectiveView = (PerspectiveView *)data;
		writer.writeData(&perspectiveView->aspect, sizeof(perspectiveView->aspect));
		writer.writeData(&perspectiveView->fov, sizeof(perspectiveView->fov));
		writer.writeData(&perspectiveView->nearPlane, sizeof(perspectiveView->nearPlane));
		writer.writeData(&perspectiveView->farPlane, sizeof(perspectiveView->farPlane));
	});

	registerAssetReferenceSerialization<Mesh>();
	registerAssetReferenceSerialization<Material>();
	registerAssetReferenceSerialization<Shader>();

	registerDeserializationMethod<int>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(int));
	});
	registerDeserializationMethod<UUID>([](StreamReader &reader, void *data) {
		uint32_t rawID;
		reader.readData(&rawID, sizeof(uint32_t));
		static_cast<uint32_t *>(data)[0] = rawID;
	});
	registerDeserializationMethod<uint64_t>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(uint64_t));
	});
	registerDeserializationMethod<uint32_t>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(uint32_t));
	});
	registerDeserializationMethod<float>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(float));
	});
	registerDeserializationMethod<double>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(double));
	});
	registerDeserializationMethod<std::string>([](StreamReader &reader, void *data) {
		std::string *str = (std::string *)data;
		reader.readString(*str);
	});
	registerDeserializationMethod<glm::vec2>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec2));
	});
	registerDeserializationMethod<glm::vec3>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec3));
	});
	registerDeserializationMethod<glm::vec4>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::vec4));
	});
	registerDeserializationMethod<glm::mat4>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::mat4));
	});
	registerDeserializationMethod<glm::quat>([](StreamReader &reader, void *data) {
		reader.readData(data, sizeof(glm::quat));
	});
	registerDeserializationMethod<PerspectiveView>([](StreamReader &reader, void *data) {
		PerspectiveView *perspectiveView = (PerspectiveView *)data;
		reader.readData(&perspectiveView->aspect, sizeof(perspectiveView->aspect));
		reader.readData(&perspectiveView->fov, sizeof(perspectiveView->fov));
		reader.readData(&perspectiveView->nearPlane, sizeof(perspectiveView->nearPlane));
		reader.readData(&perspectiveView->farPlane, sizeof(perspectiveView->farPlane));
	});
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
	registerComponentMember<PerspectiveCameraComponent, PerspectiveView>("view", offset_of<&PerspectiveCameraComponent::view, PerspectiveView>());
}

void ECSRegistry::registerDefaultSystems() {
}
