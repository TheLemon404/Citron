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
		const uint32_t systemHash = Hashing::typeHash<T>();
		SystemMetadata metadata;
		metadata.hash = systemHash;
		metadata.name = name;

		metadata.add = [](std::shared_ptr<Scene> scene) {
			scene->addSystem<T>();
		};
		metadata.remove = [](std::shared_ptr<Scene> scene) {
			scene->removeSystem<T>();
		};
		metadata.get = [](std::shared_ptr<Scene> scene) {
			return scene->getSystem<T>();
		};
		metadata.has = [](std::shared_ptr<Scene> scene) {
			return scene->hasSystem<T>();
		};

		m_systemRegistry[systemHash] = metadata;
	}

	template <typename T>
	static void registerComponent(std::string name) {
		const uint32_t componentHash = Hashing::typeHash<T>();
		ComponentMetadata metadata;
		metadata.hash = componentHash;
		metadata.name = name;
		metadata.has = [](entt::registry &registry, entt::entity entity) {
			return registry.any_of<T>(entity);
		};
		metadata.add = [](entt::registry &registry, entt::entity entity) {
			registry.emplace<T>(entity);
		};
		metadata.remove = [](entt::registry &registry, entt::entity entity) {
			registry.remove<T>(entity);
		};
		metadata.get = [](entt::registry &registry, entt::entity entity) {
			return &registry.get<T>(entity);
		};

		m_componentRegistry[componentHash] = metadata;
	}

	template <typename T>
	static const ComponentMetadata &getComponentMetadata() {
		return m_componentRegistry.at(Hashing::typeHash<T>());
	}

	template <typename T, typename U>
	static void registerComponentMember(std::string memberName, size_t offset, bool hideInEditor = false) {
		uint32_t parentClassTypeHash = Hashing::typeHash<T>();
		uint32_t memberTypeHashCode = Hashing::typeHash<U>();

		if(!Member::serializationMethods.contains(memberTypeHashCode) || !Member::deserializationMethods.contains(memberTypeHashCode)) {
			CITRON_CORE_CRITICAL("no property serialization or deserialization registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property serialization or deserialization registered for component " + std::string(typeid(T).name()));
		}

		Member member = {};
		member.fieldName = memberName;
		member.typeInfo = &typeid(U);
		member.offset = offset;
		member.serialize = Member::serializationMethods[memberTypeHashCode];
		member.deserialize = Member::deserializationMethods[memberTypeHashCode];
		member.hideInEditor = hideInEditor;

		if (m_propertyGuiDrawers.contains(memberTypeHashCode)) {
			member.drawer = m_propertyGuiDrawers[memberTypeHashCode];
		} else if (!hideInEditor) {
			CITRON_CORE_CRITICAL("no property gui drawer registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property gui drawer registered for component " + std::string(typeid(T).name()));
		}

		m_componentRegistry[parentClassTypeHash].members.emplace_back(member);
	}

	template <typename T, typename U>
	static void registerSystemMember(std::string memberName, size_t offset, bool hideInEditor = false) {
		uint32_t parentClassTypeHash = Hashing::typeHash<T>();
		uint32_t memberTypeHashCode = Hashing::typeHash<U>();

		if(!Member::serializationMethods.contains(memberTypeHashCode) || !Member::deserializationMethods.contains(memberTypeHashCode)) {
			CITRON_CORE_CRITICAL("no property serialization or deserialization registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property serialization or deserialization registered for component " + std::string(typeid(T).name()));
		}

		Member member = {};
		member.fieldName = memberName;
		member.typeInfo = &typeid(U);
		member.offset = offset;
		member.serialize = Member::serializationMethods[memberTypeHashCode];
		member.deserialize = Member::deserializationMethods[memberTypeHashCode];
		member.hideInEditor = hideInEditor;

		if (m_propertyGuiDrawers.contains(memberTypeHashCode)) {
			member.drawer = m_propertyGuiDrawers[memberTypeHashCode];
		} else if (!hideInEditor) {
			CITRON_CORE_CRITICAL("no property gui drawer registered for component {} {}", memberName, typeid(T).name());
			throw std::runtime_error("no property gui drawer registered for component " + std::string(typeid(T).name()));
		}

		m_systemRegistry[parentClassTypeHash].members.emplace_back(member);
	}

	template <typename T>
	static void registerPropertyGuiDrawer(PropertyGuiDrawer drawer) {
		m_propertyGuiDrawers[Hashing::typeHash<T>()] = drawer;
	}

	template <typename T>
	static void registerCollectionSerialization() {
		Member::serializationMethods[Hashing::typeHash<std::vector<T>>()] = [](StreamWriter &writer, void *data) {
			std::vector<T> *vec = static_cast<std::vector<T> *>(data);
			size_t vectorSize = vec->size();
			writer.writeData(&vectorSize, sizeof(size_t));
			for (T &item : *vec) {
				Member::serializationMethods[Hashing::typeHash<T>()](writer, (void *)&item);
			}
		};
	}
	template <typename T>
	static void registerCollectionDeserialization() {
		Member::deserializationMethods[Hashing::typeHash<std::vector<T>>()] = [](StreamReader &reader, void *data) {
			std::vector<T> *vec = static_cast<std::vector<T> *>(data);
			size_t size;
			reader.readData(&size, sizeof(size_t));
			vec->resize(size);
			for (T &item : *vec) {
				Member::deserializationMethods[Hashing::typeHash<T>()](reader, (void *)&item);
			}
		};
	}


	template <typename T>
	static void registerSerializationMethod(std::function<void(StreamWriter &, void *)> serialize) {
		Member::serializationMethods[Hashing::typeHash<T>()] = serialize;
		registerCollectionSerialization<T>();
	}

	template <typename T>
	static void registerDeserializationMethod(std::function<void(StreamReader &, void *)> deserialize) {
		Member::deserializationMethods[Hashing::typeHash<T>()] = deserialize;
		registerCollectionDeserialization<T>();
	}

	template <typename T>
	static void registerAssetReferenceSerialization() {
		Member::serializationMethods[Hashing::typeHash<AssetReference<T>>()] = [](StreamWriter &writer, void *data) {
			AssetReference<T> *ref = static_cast<AssetReference<T> *>(data);
			writer.writeData(&ref->uuid, sizeof(uint32_t));
			writer.writeString(ref->path);
		};
	};
	template <typename T>
	static void registerAssetReferenceDeserialization() {
		Member::deserializationMethods[Hashing::typeHash<AssetReference<T>>()] = [](StreamReader &reader, void *data) {
			AssetReference<T> *ref = static_cast<AssetReference<T> *>(data);
			reader.readData(&ref->uuid, sizeof(uint32_t));
			reader.readString(ref->path);
		};
	}

	static void registerDefaultComponents();
	static void registerDefaultSystems();
};

} // namespace CitronECS
