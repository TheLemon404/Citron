#include "ecs.hpp"
#include "component.hpp"
#include "entt/entity/fwd.hpp"
#include "logger.hpp"
#include "serialization.hpp"
#include "uuid.hpp"
#include <io.hpp>
#include <memory>

using namespace CitronECS;

void Scene::serialize(StreamWriter &writer) {
	writer.writeString(name);
	entt::snapshot{registry}.get<entt::entity>(writer).get<EntityBaseComponent>(
		writer);
}

void Scene::deserialize(StreamReader &reader) {
	reader.readString(name);
	registry.clear();
	entt::snapshot_loader{registry}
		.get<entt::entity>(reader)
		.get<EntityBaseComponent>(reader)
		.orphans();
	for (auto [entity, baseComponent] :
		 registry.view<EntityBaseComponent>().each()) {
		entityMap[baseComponent.uuid] = entity;
	}
}

UUID Scene::createEntity() {
	const auto entity = registry.create();
	UUID uuid = UUID();
	registry.emplace<EntityBaseComponent>(entity, uuid, "Entity");
	entityMap[uuid] = entity;

	CITRON_CORE_INFO("Successfully created entity: {}", (int)uuid);

	return uuid;
}

entt::entity Scene::getEntity(UUID uuid) { return entityMap[uuid]; }

template <typename T>
void Scene::addComponent(entt::entity entity, T component) {
	registry.emplace<T>(entity, component);
}

void Scene::reparentEntity(entt::entity entity, entt::entity parent) {
	EntityBaseComponent &newParentBase =
		registry.get<EntityBaseComponent>(parent);
	EntityBaseComponent &base = registry.get<EntityBaseComponent>(entity);

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

void Scene::deleteEntity(entt::entity entity) {
	EntityBaseComponent &base = registry.get<EntityBaseComponent>(entity);

	UUID uuid = base.uuid;
	for (UUID childID : base.children) {
		deleteEntity(childID);
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

void Scene::deleteEntity(UUID uuid) {
	entt::entity e = entityMap[uuid];
	deleteEntity(e);
}

void Scene::init() {
	for (auto &system : systems) {
		system->init(*this);
	}
}

void Scene::start() {
	for (auto &system : systems) {
		system->start(*this);
	}
}

void Scene::update() {
	for (auto &system : systems) {
		system->update(*this);
	}
}

void Scene::editorUpdate() {}

void Scene::onEvent(Event &e) {
	for (auto &system : systems) {
		system->onEvent(*this, e);
	}
}

void Scene::end() {
	for (auto &system : systems) {
		system->end(*this);
	}
}

void SceneLayer::switchScene(std::shared_ptr<Scene> newScene) {
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

void SceneLayer::onAttach() {}

void SceneLayer::onDetach() {}

void SceneLayer::onUpdate() {
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

void SceneLayer::onEvent(Event &e) {
	if (activeScene) {
		if (mode == SceneMode::PLAY)
			activeScene->onEvent(e);
	}
}
