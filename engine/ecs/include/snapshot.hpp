#pragma once

#include "ecs.hpp"
#include "registry.hpp"
#include "serialization.hpp"
#include <sstream>
#include <streambuf>

namespace CitronECS {
	struct PerEntitySnapshotData {
		entt::entity handle;
		std::unordered_map<uint32_t, ComponentMetadata> componentMetadata;
	};

	class CITRON_ECS_API EntitySnapshot : public ISerializable  {
		PerEntitySnapshotData entity;
		std::shared_ptr<Scene> parentScene;
		std::vector<EntitySnapshot> children;
		UUID parentId;

		std::vector<uint8_t> buffer;
	
  public:
		EntitySnapshot(StreamReader &reader, std::shared_ptr<Scene> parentScene, UUID parentId = UUID::nullID, bool restoreData = false);
		EntitySnapshot(Entity entity, std::shared_ptr<Scene> parentScene);
	
		virtual void serialize(StreamWriter &writer) override;
		virtual void deserialize(StreamReader &reader) override;
		//essencially deserialize, but without randomizing the new entity's UUID. Primarily used for restoring snapshots.
		void restore(StreamReader& reader);
		void apply();
	};
}
