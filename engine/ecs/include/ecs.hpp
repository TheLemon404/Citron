#pragma once

#include "entt/entity/fwd.hpp"
#include "serialization.hpp"
#include "uuid.hpp"
#include <assets.hpp>
#include <entt/entt.hpp>
#include <layer.hpp>
#include <map>
#include <memory>

using namespace CitronAssets;
using namespace CitronCore;

namespace CitronECS {

class Scene;

struct EntityBase {
	uint64_t uuid;
	std::string name;
};

class System {
  public:
	System(const std::string name) : name(name) {}
	virtual void init(Scene &activeScene) {};
	virtual void start(Scene &activeScene) {};
	virtual void update(Scene &activeScene) {};
	virtual void onEvent(Scene &activeScene, Event &e) {};
	virtual void end(Scene &registry) {};
	virtual const std::string getName() { return name; }

  private:
	const std::string name;
};

class Scene : public ISerializable<Scene> {
  public:
	Scene(StreamReader &reader) { deserialize(reader); }
	Scene(std::string name) : name(name) {}

	virtual std::string serialize(StreamWriter &writer) const override;
	virtual void deserialize(StreamReader &reader) override;

	const std::string &getName() { return name; }
	std::vector<std::shared_ptr<System>> &getSystems() { return systems; }
	entt::registry &getRegistry() { return registry; }

	void createEntity();
	entt::entity getEntity(UUID uuid);
	void deleteEntity(UUID uuid);
	void deleteEntity(entt::entity entity);

	void rename(const std::string &name) { this->name = name; }

	void init();
	void start();
	void update();
	void editorUpdate();
	void onEvent(Event &e);
	void end();
	std::vector<std::shared_ptr<System>> systems;

  private:
	std::map<UUID, entt::entity> entityMap;

	std::string name;
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

	std::shared_ptr<Scene> &getActiveScene() { return activeScene; }

  private:
	SceneMode mode = SceneMode::EDIT;
	std::shared_ptr<Scene> activeScene;
};
} // namespace CitronECS
