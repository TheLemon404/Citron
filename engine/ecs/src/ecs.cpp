#include "ecs.hpp"
#include <memory>

using namespace CitronECS;

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
