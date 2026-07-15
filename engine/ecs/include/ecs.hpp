#pragma once

#include "entt/entity/fwd.hpp"
#include <entt/entt.hpp>
#include <layer.hpp>
#include <memory>

using namespace CitronCore;

namespace CitronECS {

class Scene;

class System {
public:
  virtual void init(Scene &activeScene) = 0;
  virtual void start(Scene &activeScene) = 0;
  virtual void update(Scene &activeScene) = 0;
  virtual void onEvent(Scene &activeScene, Event &e) = 0;
  virtual void end(Scene &registry) = 0;
};

class Scene {
public:
  void init();
  void start();
  void update();
  void onEvent(Event &e);
  void end();

private:
  std::vector<System> systems;
  entt::registry registry;
};

class SceneLayer : public Layer {
public:
  SceneLayer() : Layer("SceneLayer") {}
  ~SceneLayer() override = default;

  void onAttach() override;
  void onDetach() override;
  void onUpdate() override;
  void onEvent(Event &e) override;

  void switchScene(std::shared_ptr<Scene> newScene);

private:
  std::shared_ptr<Scene> activeScene;
};
} // namespace CitronECS
