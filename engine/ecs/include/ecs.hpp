#pragma once

#include "citron_exports.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "serialization.hpp"
#include "uuid.hpp"
#include <assets.hpp>
#include <cstdint>
#include <entt/entt.hpp>
#include <layer.hpp>
#include <map>
#include <memory>
#include <resources.hpp>

using namespace CitronAssets;
using namespace CitronCore;

namespace CitronECS {

class CITRON_ECS_API Scene;

class CITRON_ECS_API System {
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

class CITRON_ECS_API Scene : public ISerializable {
  public:
	Scene(std::string name) : name(name) {}

	virtual void serialize(StreamWriter &writer) override;
	virtual void deserialize(StreamReader &reader) override;

	const std::string &getName() { return name; }
	std::vector<std::shared_ptr<System>> &getSystems() { return systems; }
	entt::registry &getRegistry() { return registry; }

	UUID createEntity();
	entt::entity getEntity(UUID uuid);
	template <typename T>
	void addComponent(entt::entity entity, T component);
	void reparentEntity(entt::entity entity, entt::entity parent);
	void deleteEntity(UUID uuid);
	void deleteEntity(entt::entity entity);

	std::vector<CitronGraphics::RenderableReferenceData> extractRenderableData(AssetManager &assetManager);

	std::vector<uint64_t> extractDrawableEntityUUIDs();
	std::vector<glm::mat4> extractDrawableEntityTransforms();
	std::vector<uint64_t> extractMeshes(AssetManager &assetManager);
	std::vector<uint64_t> extractMaterials(AssetManager &assetManager);

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

class CITRON_ECS_API SceneManager {
  public:
	SceneManager(AssetManager &assetManager) : assetManager(assetManager) {}
	void onAttach();
	void onDetach();
	void onUpdate();
	void onEvent(Event &e);

	void switchScene(std::shared_ptr<Scene> newScene);

	std::shared_ptr<Scene> getActiveScene() { return activeScene; }
	void setActiveScene(std::shared_ptr<Scene> newScene);

  private:
	bool checkAllAssetReferenceValidity(AssetRegistryRefreshEvent &e);

	AssetManager &assetManager;
	SceneMode mode = SceneMode::EDIT;
	std::shared_ptr<Scene> activeScene;
};
} // namespace CitronECS
