
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/vector_float3.hpp"
#include "registry.hpp"
#include "view.hpp"
#include <stdexcept>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "assets.hpp"
#include "component.hpp"
#include "core.hpp"
#include "ecs.hpp"
#include "event.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "logger.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "serialization.hpp"
#include "shader.hpp"
#include "uuid.hpp"
#include <io.hpp>
#include <memory>
#include <renderer.hpp>

using namespace CitronECS;

std::vector<Entity> Entity::getChildren() {
	EntityBaseComponent &base = getComponent<EntityBaseComponent>();
	std::vector<Entity> result = {};
	for (UUID childID : base.children) {
		result.push_back({scene->getEntity(childID), scene});
	}
	return result;
}

Entity Entity::getParent() {
	EntityBaseComponent &base = getComponent<EntityBaseComponent>();
	return {scene->getEntity(base.parentId), scene};
}

void Scene::serialize(StreamWriter &writer) {
	writer.writeString(name);
	size_t numSystems = m_systemRegistry.size();
	writer.writeData(&numSystems, sizeof(numSystems));
	for (const auto &[typeHash, metadata] : ECSRegistry::getSystemRegistry()) {
		if (metadata.has(shared_from_this())) {
			writer.writeData(&metadata.hash, sizeof(metadata.hash));
			for (const Member &member : metadata.members) {
				void *memberBytes = (char *)m_systemRegistry[typeHash].get() + member.offset;
				member.serialize(writer, memberBytes);
			}
		}
	}

	auto view = registry.view<EntityBaseComponent>();
	size_t numEntities = view.size();
	writer.writeData(&numEntities, sizeof(numEntities));
	for (auto entity : view) {
		std::map<uint32_t, ComponentMetadata> componentsOnEntity;
		for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
			if (metadata.has(registry, entity)) {
				componentsOnEntity[typeHash] = metadata;
			}
		}
		size_t numComponents = componentsOnEntity.size();
		writer.writeData(&numComponents, sizeof(numComponents));
		for (const auto &[typeHash, metadata] : componentsOnEntity) {
			writer.writeData(&typeHash, sizeof(typeHash));
			for (const Member &member : metadata.members) {
				void *component = metadata.get(registry, entity);
				void *memberBytes = (char *)component + member.offset;
				member.serialize(writer, memberBytes);
			}
		}
	}
}

void Scene::deserialize(StreamReader &reader) {
	registry.clear();
	entityMap.clear();

	reader.readString(name);
	size_t numSystems;
	reader.readData(&numSystems, sizeof(numSystems));
	for (size_t i = 0; i < numSystems; i++) {
		uint32_t typeHash;
		reader.readData(&typeHash, sizeof(typeHash));
		SystemMetadata metadata = ECSRegistry::getSystemRegistry()[typeHash];
		metadata.add(shared_from_this());
		for (Member &member : metadata.members) {
			void *memberBytes = (char *)m_systemRegistry[typeHash].get() + member.offset;
			member.deserialize(reader, memberBytes);
		}
	}

	size_t numEntities;
	reader.readData(&numEntities, sizeof(numEntities));
	for (size_t i = 0; i < numEntities; i++) {
		size_t numComponents;
		entt::entity entity = registry.create();
		reader.readData(&numComponents, sizeof(numComponents));
		for (size_t j = 0; j < numComponents; j++) {
			uint32_t typeHash;
			reader.readData(&typeHash, sizeof(typeHash));
			ComponentMetadata metadata = ECSRegistry::getComponentRegistry()[typeHash];
			metadata.add(registry, entity);
			for (Member &member : metadata.members) {
				void *component = metadata.get(registry, entity);
				void *memberBytes = (char *)component + member.offset;
				member.deserialize(reader, memberBytes);
			}
		}
		uint32_t uuid = registry.get<EntityBaseComponent>(entity).uuid;
		entityMap[uuid] = entity;
	}
}

Entity Scene::createEntity() {
	const auto entity = registry.create();
	UUID uuid = UUID();
	registry.emplace<EntityBaseComponent>(entity, uuid, "Entity");
	registry.emplace<TransformComponent>(entity);
	entityMap[uuid] = entity;

	CITRON_CORE_INFO("Successfully created entity: {}", (int)uuid);

	return {entity, this};
}

Entity Scene::getEntity(UUID uuid) {
	if (entityMap.contains(uuid)) {
		return {entityMap[uuid], this};
	}
	CITRON_CORE_ERROR("Entity not found: {}", (unsigned int)uuid);
	throw std::runtime_error("Entity not found: " + std::to_string(uuid));
}

void Scene::reparentEntityToRoot(Entity entity) {
	EntityBaseComponent &base = registry.get<EntityBaseComponent>(entity);

	if (base.parentId != UUID::nullID) {
		EntityBaseComponent &oldParentBase =
			registry.get<EntityBaseComponent>(entityMap[base.parentId]);
		oldParentBase.children.erase(std::remove(oldParentBase.children.begin(),
												 oldParentBase.children.end(),
												 base.uuid),
									 oldParentBase.children.end());
	}

	base.parentId = UUID::nullID;
}

void Scene::reparentEntity(Entity entity, Entity parent) {
	EntityBaseComponent &base = registry.get<EntityBaseComponent>(entity);

	EntityBaseComponent &newParentBase =
		registry.get<EntityBaseComponent>(parent);

	if (base.uuid == newParentBase.uuid || base.parentId == newParentBase.uuid)
		return;

	if (base.parentId != UUID::nullID) {
		EntityBaseComponent &oldParentBase =
			registry.get<EntityBaseComponent>(entityMap[base.parentId]);
		oldParentBase.children.erase(std::remove(oldParentBase.children.begin(),
												 oldParentBase.children.end(),
												 base.uuid),
									 oldParentBase.children.end());
	}

	base.parentId = newParentBase.uuid;
	newParentBase.children.push_back(base.uuid);

	CITRON_CORE_INFO("Successfully reparented entity: {} to parent: {}",
					 (unsigned int)base.uuid, (unsigned int)newParentBase.uuid);
}

void Scene::deleteEntity(Entity entity) {
	EntityBaseComponent &base = registry.get<EntityBaseComponent>(entity);

	UUID uuid = base.uuid;
	for (Entity child : entity.getChildren()) {
		deleteEntity(child);
	}

	if (base.parentId != UUID::nullID) {
		EntityBaseComponent &parentBase =
			registry.get<EntityBaseComponent>(entityMap[base.parentId]);
		parentBase.children.erase(std::remove(parentBase.children.begin(),
											  parentBase.children.end(), uuid),
								  parentBase.children.end());
	}

	CITRON_CORE_INFO("Successfully deleted entity: {}", (int)base.uuid);

	registry.destroy(entity);
	entityMap.erase(uuid);
}

glm::vec3 Scene::getGlobalPosition(entt::entity entity) {
	if (!registry.any_of<TransformComponent>(entity)) {
		return glm::vec3(0.0f);
	}

	TransformComponent &t = registry.get<TransformComponent>(entity);
	EntityBaseComponent &b = registry.get<EntityBaseComponent>(entity);

	bool hasValidParent = b.parentId != UUID::nullID && entityMap.contains(b.parentId) && registry.any_of<TransformComponent>(entity);

	return hasValidParent ? getGlobalPosition(getEntity(b.parentId)) + t.position : t.position;
}

glm::quat Scene::getGlobalRotation(entt::entity entity) {
	if (!registry.any_of<TransformComponent>(entity)) {
		return glm::identity<glm::quat>();
	}

	TransformComponent &t = registry.get<TransformComponent>(entity);
	EntityBaseComponent &b = registry.get<EntityBaseComponent>(entity);

	bool hasValidParent = b.parentId != UUID::nullID && entityMap.contains(b.parentId) && registry.any_of<TransformComponent>(entity);

	return hasValidParent ? getGlobalRotation(getEntity(b.parentId)) * t.rotation : t.rotation;
}

glm::vec3 Scene::getGlobalScale(entt::entity entity) {
	if (!registry.any_of<TransformComponent>(entity)) {
		return glm::vec3(1.0f);
	}

	TransformComponent &t = registry.get<TransformComponent>(entity);
	EntityBaseComponent &b = registry.get<EntityBaseComponent>(entity);

	bool hasValidParent = b.parentId != UUID::nullID && entityMap.contains(b.parentId) && registry.any_of<TransformComponent>(entity);

	return hasValidParent ? getGlobalScale(getEntity(b.parentId)) * t.scale : t.scale;
}

glm::mat4 Scene::getGlobalTransform(entt::entity entity) {
	if (!registry.any_of<TransformComponent>(entity)) {
		return glm::identity<glm::mat4>();
	}

	TransformComponent &t = registry.get<TransformComponent>(entity);
	EntityBaseComponent &b = registry.get<EntityBaseComponent>(entity);

	t.matrix = glm::identity<glm::mat4>();
	t.matrix = glm::translate(glm::mat4(1.0f), t.position) *
			   glm::mat4_cast(glm::normalize(t.rotation)) *
			   glm::scale(glm::mat4(1.0f), t.scale);

	bool hasValidParent = b.parentId != UUID::nullID && entityMap.contains(b.parentId) && registry.any_of<TransformComponent>(entity);

	return hasValidParent ? getGlobalTransform(getEntity(b.parentId)) * t.matrix : t.matrix;
}

std::vector<CitronGraphics::RenderableReferenceData> Scene::extractRenderableData(AssetManager &assetManager) {
	std::vector<CitronGraphics::RenderableReferenceData> renderableData;
	for (auto &entity : registry.view<MeshComponent, TransformComponent, EntityBaseComponent>()) {
		CitronGraphics::RenderableReferenceData data;
		data.entityUUID = registry.get<EntityBaseComponent>(entity).uuid;

		data.transform = getGlobalTransform(entity);

		MeshComponent &meshComponent = registry.get<MeshComponent>(entity);
		if (!assetManager.isValidAsset(meshComponent.meshAsset.uuid))
			continue;
		if (!assetManager.isValidAsset(meshComponent.materialAsset.uuid))
			continue;

		data.meshUUID = meshComponent.meshAsset.uuid;
		data.materialUUID = meshComponent.materialAsset.uuid;
		renderableData.push_back(data);
	}
	return renderableData;
}

void Scene::init() {
	for (auto &[id, system] : m_systemRegistry) {
		system->init(*this);
	}
}

void Scene::start() {
	for (auto &[id, system] : m_systemRegistry) {
		system->start(*this);
	}
}

void Scene::update() {
	for (auto &[id, system] : m_systemRegistry) {
		system->update(*this);
	}
}

void Scene::editorUpdate() {}

void Scene::onEvent(Event &e) {
	for (auto &[id, system] : m_systemRegistry) {
		system->onEvent(*this, e);
	}
}

void Scene::end() {
	for (auto &[id, system] : m_systemRegistry) {
		system->end(*this);
	}
}

View &Scene::getActiveView() {
	const auto &cameraComponentView = registry.view<PerspectiveCameraComponent, TransformComponent>();
	for (auto [entity, cameraComponent, transformComponent] : cameraComponentView.each()) {
		cameraComponent.view.position = transformComponent.position;
		cameraComponent.view.forward = transformComponent.rotation * glm::vec3(0.0f, 0.0f, 1.0f);
		return cameraComponent.view;
	}
	return tempSceneView;
}

void SceneManager::switchScene(std::shared_ptr<Scene> newScene) {
	if (activeScene) {
		if (mode == SceneMode::PLAY)
			activeScene->end();
	}
	activeScene = newScene;
	if (mode == SceneMode::PLAY) {
		activeScene->init();
		activeScene->start();
	}
}

void SceneManager::onAttach() {}

void SceneManager::onDetach() {}

void SceneManager::onUpdate() {
	if (activeScene) {
		switch (mode) {
		case SceneMode::EDIT:
			activeScene->editorUpdate();
			break;
		case SceneMode::PLAY:
			activeScene->update();
			break;
		}
	}
}

void SceneManager::onEvent(Event &e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<AssetRegistryRefreshEvent>(CITRON_BIND_EVENT_FN(SceneManager::checkAllAssetReferenceValidity));
	if (activeScene) {
		if (mode == SceneMode::PLAY)
			activeScene->onEvent(e);
	}
}

void SceneManager::setActiveScene(std::shared_ptr<Scene> newScene) {
	activeScene = newScene;
}

bool SceneManager::checkAllAssetReferenceValidity(AssetRegistryRefreshEvent &e) {
	entt::registry &registry = activeScene->getRegistry();
	for (auto &entity : registry.view<MeshComponent>()) {
		MeshComponent &comp = registry.get<MeshComponent>(entity);
		if (!assetManager.isValidAsset(comp.meshAsset.uuid)) {
			comp.meshAsset.uuid = UUID::nullID;
		}
		if (!assetManager.isValidAsset(comp.materialAsset.uuid)) {
			comp.materialAsset.uuid = UUID::nullID;
		}
	}
	return true;
}
