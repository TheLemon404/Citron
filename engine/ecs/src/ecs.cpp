#include "ecs.hpp"
#include <memory>

using namespace CitronECS;

void Scene::init() {
  for (auto &system : systems) {
    system.init(*this);
  }
}

void Scene::start() {
  for (auto &system : systems) {
    system.start(*this);
  }
}

void Scene::update() {
  for (auto &system : systems) {
    system.update(*this);
  }
}

void Scene::onEvent(Event &e) {
  for (auto &system : systems) {
    system.onEvent(*this, e);
  }
}

void Scene::end() {
  for (auto &system : systems) {
    system.end(*this);
  }
}

void SceneLayer::switchScene(std::shared_ptr<Scene> newScene) {
	if(activeScene) {
		activeScene->end();
	}
	activeScene = newScene;
	activeScene->init();
	activeScene->start();
}

void SceneLayer::onAttach() {}

void SceneLayer::onDetach() {}

void SceneLayer::onUpdate() {
	if(activeScene) {
		activeScene->update();
	}
}

void SceneLayer::onEvent(Event &e) {
	if(activeScene) {
		activeScene->onEvent(e);
	}
}
