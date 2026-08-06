#pragma once

#include "citron_exports.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "logger.hpp"
#include "serialization.hpp"
#include "uuid.hpp"
#include "view.hpp"
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
	const std::string getName() { return name; }

  private:
	const std::string name;
};

class CITRON_ECS_API Entity;

class CITRON_ECS_API Scene : public ISerializable {
  public:
	Scene(std::string name) : name(name) {}

	virtual void serialize(StreamWriter &writer) override;
	virtual void deserialize(StreamReader &reader) override;

	template <typename T>
	void addSystem() {
		if (!hasSystem<T>()) {
			m_systemRegistry[typeid(T).hash_code()] = std::make_shared<T>();
		}
	}

	template <typename T>
	void removeSystem() {
		if (hasSystem<T>()) {
			m_systemRegistry.erase(typeid(T).hash_code());
		}
	}

	template <typename T>
	bool hasSystem() {
		return m_systemRegistry.contains(typeid(T).hash_code());
	}

	template <typename T>
	std::shared_ptr<T> getSystem() {
		if (hasSystem<T>()) {
			return std::static_pointer_cast<T>(m_systemRegistry[typeid(T).hash_code()]);
		}
		return nullptr;
	}

	const std::string &getName() { return name; }
	std::map<uint32_t, std::shared_ptr<System>> &getSystems() { return m_systemRegistry; }
	entt::registry &getRegistry() { return registry; }

	Entity createEntity();
	Entity getEntity(UUID entity);
	void reparentEntityToRoot(Entity entity);
	void reparentEntity(Entity entity, Entity parent);
	void deleteEntity(Entity entity);

	glm::vec3 getGlobalPosition(entt::entity entity);
	glm::quat getGlobalRotation(entt::entity entity);
	glm::vec3 getGlobalScale(entt::entity entity);
	glm::mat4 getGlobalTransform(entt::entity entity);

	std::vector<CitronGraphics::RenderableReferenceData>
	extractRenderableData(AssetManager &assetManager);

	void rename(const std::string &name) { this->name = name; }

	void init();
	void start();
	void update();
	void editorUpdate();
	void onEvent(Event &e);
	void end();

	CitronGraphics::View &getActiveView() { return tempPerspectiveView; }

  private:
	// needs to be swapped out later with current scene camera
	CitronGraphics::PerspectiveView tempPerspectiveView;

	std::map<UUID, entt::entity> entityMap;

	std::string name;
	entt::registry registry;
	std::map<uint32_t, std::shared_ptr<System>> m_systemRegistry;
};

class CITRON_ECS_API Entity {
  public:
	Entity(const entt::entity handle, Scene *scene) : handle(handle), scene(scene) {};

	template <typename T>
	bool hasComponent() {
		return scene->getRegistry().any_of<T>(handle);
	}

	template <typename T>
	T &getComponent() {
		return scene->getRegistry().get<T>(handle);
	}

	template <typename T>
	void removeComponent() {
		if (hasComponent<T>())
			scene->getRegistry().remove<T>(handle);
	}

	template <typename T, typename... Args>
	void addComponent(Args... args) {
		if (!hasComponent<T>())
			scene->getRegistry().emplace<T>(handle, args...);
	}

	Entity getParent();
	std::vector<Entity> getChildren();

	operator entt::entity() { return handle; }

  private:
	const entt::entity handle;
	Scene *scene;
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
