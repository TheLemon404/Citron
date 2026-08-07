#pragma once

#include "assets.hpp"
#include "citron_exports.hpp"
#include "ecs.hpp"
#include "serialization.hpp"
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
	std::function<void(StreamWriter &, void *)> serialize;
	std::function<void(StreamReader &, void *)> deserialize;
	bool hideInEditor = false;
	PropertyGuiDrawer drawer;

	static std::unordered_map<uint32_t, std::function<void(StreamWriter &, void *)>> serializationMethods;
	static std::unordered_map<uint32_t, std::function<void(StreamReader &, void *)>> deserializationMethods;
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
	static void registerComponentMember(std::string memberName, size_t offset, bool hideInEditor = false) {
		uint32_t parentClassTypeHash = typeid(T).hash_code();
		uint32_t memberTypeHashCode = typeid(U).hash_code();
		if (m_propertyGuiDrawers.contains(memberTypeHashCode)) {
			Member member = {};
			member.fieldName = memberName;
			member.typeInfo = &typeid(U);
			member.offset = offset;
			member.drawer = m_propertyGuiDrawers[memberTypeHashCode];
			member.serialize = Member::serializationMethods[memberTypeHashCode];
			member.deserialize = Member::deserializationMethods[memberTypeHashCode];
			member.hideInEditor = hideInEditor;
			m_componentRegistry[parentClassTypeHash].members.emplace_back(member);
		} else if (!hideInEditor) {
			CITRON_CORE_CRITICAL("no property gui drawer registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property gui drawer registered for component " + std::string(typeid(T).name()));
		}
	}

	template <typename T, typename U>
	static void registerSystemMember(std::string memberName, size_t offset, bool hideInEditor = false) {
		uint32_t parentClassTypeHash = typeid(T).hash_code();
		uint32_t memberTypeHashCode = typeid(U).hash_code();
		if (m_propertyGuiDrawers.contains(memberTypeHashCode)) {
			Member member = {};
			member.fieldName = memberName;
			member.typeInfo = &typeid(U);
			member.offset = offset;
			member.drawer = m_propertyGuiDrawers[memberTypeHashCode];
			member.serialize = Member::serializationMethods[memberTypeHashCode];
			member.deserialize = Member::deserializationMethods[memberTypeHashCode];
			member.hideInEditor = hideInEditor;
			m_systemRegistry[parentClassTypeHash].members.emplace_back(member);
		} else if (!hideInEditor) {
			CITRON_CORE_CRITICAL("no property gui drawer registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property gui drawer registered for component " + std::string(typeid(T).name()));
		}
	}

	template <typename T>
	static void registerPropertyGuiDrawer(PropertyGuiDrawer drawer) {
		m_propertyGuiDrawers[typeid(T).hash_code()] = drawer;
	}

	template <typename T>
	static void registerCollectionSerialization() {
		Member::serializationMethods[typeid(std::vector<T>).hash_code()] = [](StreamWriter &writer, void *data) {
			std::vector<T> *vec = static_cast<std::vector<T> *>(data);
			size_t vectorSize = vec->size();
			writer.writeData(&vectorSize, sizeof(size_t));
			for (T &item : *vec) {
				Member::serializationMethods[typeid(T).hash_code()](writer, (void *)&item);
			}
		};
	}
	template <typename T>
	static void registerCollectionDeserialization() {
		Member::deserializationMethods[typeid(std::vector<T>).hash_code()] = [](StreamReader &reader, void *data) {
			std::vector<T> *vec = static_cast<std::vector<T> *>(data);
			size_t size;
			reader.readData(&size, sizeof(size_t));
			vec->resize(size);
			for (T &item : *vec) {
				Member::deserializationMethods[typeid(T).hash_code()](reader, (void *)&item);
			}
		};
	}

	template <typename T>
	static void registerAssetReferenceSerialization() {
		Member::serializationMethods[typeid(AssetReference<T>).hash_code()] = [](StreamWriter &writer, void *data) {
			AssetReference<T> *ref = static_cast<AssetReference<T> *>(data);
			writer.writeData(&ref->uuid, sizeof(uint32_t));
			writer.writeString(ref->path);
		};
	};
	template <typename T>
	static void registerAssetReferenceDeserialization() {
		Member::deserializationMethods[typeid(AssetReference<T>).hash_code()] = [](StreamReader &reader, void *data) {
			AssetReference<T> *ref = static_cast<AssetReference<T> *>(data);
			reader.readData(&ref->uuid, sizeof(uint32_t));
			reader.readString(ref->path);
		};
	}

	static void registerDefaultComponents();
	static void registerDefaultSystems();
};

} // namespace CitronECS
