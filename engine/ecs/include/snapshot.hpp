#pragma once

#include "ecs.hpp"
#include "registry.hpp"

namespace CitronECS {
	struct PerEntitySnapshotData {
		entt::entity handle;
		std::unordered_map<uint32_t, ComponentMetadata> componentMetadata;
		std::unordered_map<std::string, void*> memberBytes;
	};

	class CITRON_ECS_API EntitySnapshot : public ISerializable  {
		PerEntitySnapshotData entity;
		std::shared_ptr<Scene> parentScene;
		std::vector<EntitySnapshot> children;
	
  public:
		EntitySnapshot(StreamReader &reader, std::shared_ptr<Scene> parentScene);
		EntitySnapshot(Entity entity, std::shared_ptr<Scene> parentScene);
	
		virtual void serialize(StreamWriter &writer) override;
		virtual void deserialize(StreamReader &reader) override;
		void apply();
	};
}
