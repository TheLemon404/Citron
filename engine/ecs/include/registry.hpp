#pragma once

#include "assets.hpp"
#include "citron_exports.hpp"
#include "ecs.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <functional>
#include <entt/entt.hpp>

namespace CitronECS {

struct CITRON_ECS_API Member;

using PropertyGuiDrawer = std::function<void(const Member &, void *object, CitronAssets::AssetManager &)>;

struct CITRON_ECS_API Member {
	std::string fieldName;
	const std::type_info *typeInfo;
	size_t offset;
	PropertyGuiDrawer drawer;
};

struct CITRON_ECS_API ComponentMetadata {
	uint32_t hash;
	std::vector<Member> members;
	std::string name;

	std::function<bool(entt::registry &, entt::entity)> has;
	std::function<void(entt::registry &, entt::entity)> add;
	std::function<void(entt::registry &, entt::entity)> remove;
	std::function<void *(entt::registry &, entt::entity)> get;
};

struct CITRON_ECS_API SystemMetadata {
	uint32_t hash;
	std::vector<Member> members;
	std::string name;

	std::function<bool(std::shared_ptr<Scene> scene)> has;
	std::function<void(std::shared_ptr<Scene> scene)> add;
	std::function<void(std::shared_ptr<Scene> scene)> remove;
	std::function<std::shared_ptr<System>(std::shared_ptr<Scene> scene)> get;
};

class CITRON_ECS_API ECSRegistry {
	static std::unordered_map<uint32_t, ComponentMetadata> m_componentRegistry;
	static std::unordered_map<uint32_t, PropertyGuiDrawer> m_propertyGuiDrawers;
	static std::unordered_map<uint32_t, SystemMetadata> m_systemRegistry;

  public:
	static std::unordered_map<uint32_t, ComponentMetadata> &getComponentRegistry() {
		return m_componentRegistry;
	}

	static std::unordered_map<uint32_t, SystemMetadata> &getSystemRegistry() {
		return m_systemRegistry;
	}

	template <typename T>
	static void registerSystem(std::string name) {
		const uint32_t systemHash = typeid(T).hash_code();
		SystemMetadata metadata;
		metadata.hash = systemHash;
		metadata.name = name;

		metadata.add = [metadata](std::shared_ptr<Scene> scene) {
			scene->addSystem<T>();
		};
		metadata.remove = [metadata](std::shared_ptr<Scene> scene) {
			scene->removeSystem<T>();
		};
		metadata.get = [metadata](std::shared_ptr<Scene> scene) {
			return scene->getSystem<T>();
		};
		metadata.has = [metadata](std::shared_ptr<Scene> scene) {
			return scene->hasSystem<T>();
		};

		m_systemRegistry[systemHash] = metadata;
	}

	template <typename T>
	static void registerComponent(std::string name) {
		const uint32_t componentHash = typeid(T).hash_code();
		ComponentMetadata metadata;
		metadata.hash = componentHash;
		metadata.name = name;
		metadata.has = [](entt::registry &registry, entt::entity entity) {
			return registry.any_of<T>(entity);
		};
		metadata.add = [metadata](entt::registry &registry, entt::entity entity) {
			if (!metadata.has(registry, entity)) {
				registry.emplace<T>(entity);
			}
		};
		metadata.remove = [metadata](entt::registry &registry, entt::entity entity) {
			if (metadata.has(registry, entity)) {
				registry.remove<T>(entity);
			}
		};
		metadata.get = [](entt::registry &registry, entt::entity entity) {
			return &registry.get<T>(entity);
		};

		m_componentRegistry[componentHash] = metadata;
	}

	template <typename T>
	static const ComponentMetadata &getComponentMetadata() {
		return m_componentRegistry.at(typeid(T).hash_code());
	}

	template <typename T, typename U>
	static void registerComponentMember(std::string memberName, size_t offset) {
		uint32_t componentTypeHashCode = typeid(T).hash_code();
		uint32_t memberTypeHashCode = typeid(U).hash_code();
		if (m_propertyGuiDrawers.contains(memberTypeHashCode)) {
			m_componentRegistry[componentTypeHashCode].members.emplace_back(Member{memberName, &typeid(U), offset, m_propertyGuiDrawers[memberTypeHashCode]});
		} else {
			CITRON_CORE_CRITICAL("no property gui drawer registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property gui drawer registered for component " + std::string(typeid(T).name()));
		}
	}

	template <typename T>
	static void registerPropertyGuiDrawer(PropertyGuiDrawer drawer) {
		m_propertyGuiDrawers[typeid(T).hash_code()] = drawer;
	}

	static void registerDefaultComponents();
	static void registerDefaultSystems();
};

} // namespace CitronECS
