#include "ecs.hpp"
#include <cstdint>
#include <io.hpp>
#include <memory>

using namespace CitronECS;

void Scene::load(const std::string &assetSource) {}

void Scene::save(const std::string &assetPath) {
	if (!CitronIO::IO::fileExists(assetPath))
		CitronIO::IO::createFile(assetPath);

	CitronIO::IO::writeFile(assetPath, serialize());
}

std::string Scene::serialize() const { return "this is a test"; }

void Scene::deserialize(const std::string &data, Scene &result) {}

void Scene::createEntity() {
	const auto entity = registry.create();
	registry.emplace<EntityBase>(entity, static_cast<uint64_t>(0), "Entity");
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
