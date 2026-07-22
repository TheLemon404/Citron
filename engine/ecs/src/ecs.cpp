#include "ecs.hpp"
#include "entt/entity/fwd.hpp"
#include "logger.hpp"
#include "serialization.hpp"
#include "uuid.hpp"
#include <cstdint>
#include <io.hpp>
#include <memory>

using namespace CitronECS;

std::string Scene::serialize(StreamWriter &writer) const {
	return "this is a test";
}

void Scene::deserialize(StreamReader &reader) {}

UUID Scene::createEntity() {
	const auto entity = registry.create();
	UUID uuid = UUID();
	registry.emplace<EntityBase>(entity, uuid, "Entity");
	entityMap[uuid] = entity;

	CITRON_CORE_INFO("Successfully created entity: {}", (int)uuid);

	return uuid;
}

entt::entity Scene::getEntity(UUID uuid) { return entityMap[uuid]; }

void Scene::reparentEntity(entt::entity entity, entt::entity parent) {
	EntityBase &parentBase = registry.get<EntityBase>(parent);
	EntityBase &base = registry.get<EntityBase>(entity);
	base.parentId = parentBase.uuid;
	parentBase.children.push_back(base.uuid);

	CITRON_CORE_INFO("Successfully reparented entity: {} to parent: {}",
					 (int)base.uuid, (int)parentBase.uuid);
}

void Scene::deleteEntity(entt::entity entity) {
	EntityBase &base = registry.get<EntityBase>(entity);

	UUID uuid = base.uuid;
	for (UUID childID : base.children) {
		deleteEntity(childID);
	}

	EntityBase &parentBase = registry.get<EntityBase>(entityMap[base.parentId]);
	parentBase.children.erase(std::remove(parentBase.children.begin(),
										  parentBase.children.end(), uuid),
							  parentBase.children.end());

	CITRON_CORE_INFO("Successfully deleted entity: {}", (int)base.uuid);

	registry.destroy(entity);
	entityMap.erase(uuid);
}

void Scene::deleteEntity(UUID uuid) {
	entt::entity e = entityMap[uuid];
	EntityBase &base = registry.get<EntityBase>(e);
	for (UUID childID : base.children) {
		deleteEntity(childID);
	}

	EntityBase &parentBase = registry.get<EntityBase>(entityMap[base.parentId]);
	parentBase.children.erase(std::remove(parentBase.children.begin(),
										  parentBase.children.end(), uuid),
							  parentBase.children.end());

	CITRON_CORE_INFO("Successfully deleted entity: {}", (int)base.uuid);

	registry.destroy(e);
	entityMap.erase(uuid);
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
