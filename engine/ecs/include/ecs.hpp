#pragma once

#include "entt/entity/fwd.hpp"
#include <assets.hpp>
#include <entt/entt.hpp>
#include <layer.hpp>
#include <memory>

using namespace CitronAssets;
using namespace CitronCore;

namespace CitronECS {

class Scene;

class System {
  public:
	virtual void init(Scene &activeScene) {};
	virtual void start(Scene &activeScene) {};
	virtual void update(Scene &activeScene) {};
	virtual void onEvent(Scene &activeScene, Event &e) {};
	virtual void end(Scene &registry) {};
};

class Scene : public ILoadable<Scene>,
			  public ISaveable<Scene>,
			  public ISerializable<Scene> {
  public:
	Scene(std::string name) : name(name) {}

	virtual void load(const std::string &assetSource) override;
	virtual void save(const std::string &assetPath) override;
	virtual std::string serialize() const override;
	virtual void deserialize(const std::string &data, Scene &result) override;

	const std::string &getName() { return name; }

	void init();
	void start();
	void update();
	void editorUpdate();
	void onEvent(Event &e);
	void end();

  private:
	std::string name;
	std::vector<std::shared_ptr<System>> systems;
	entt::registry registry;
};

enum class SceneMode {
	EDIT = 0,
	PLAY = 1,
	PAUSE = 2,
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
	SceneMode mode = SceneMode::EDIT;
	std::shared_ptr<Scene> activeScene;
};
} // namespace CitronECS
