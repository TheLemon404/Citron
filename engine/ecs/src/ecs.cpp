#include "ecs.hpp"
#include "entt/entity/fwd.hpp"
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

void Scene::createEntity() {
	const auto entity = registry.create();
	UUID uuid = UUID();
	registry.emplace<EntityBase>(entity, uuid, "Entity");
	entityMap[uuid] = entity;
}

entt::entity Scene::getEntity(UUID uuid) { return entityMap[uuid]; }

void Scene::deleteEntity(entt::entity entity) {
	UUID uuid = registry.get<EntityBase>(entity).uuid;
	registry.destroy(entity);
	entityMap.erase(uuid);
}

void Scene::deleteEntity(UUID uuid) {
	entt::entity e = entityMap[uuid];
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
