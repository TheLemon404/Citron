#include "snapshot.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "entt/entity/fwd.hpp"
#include "serialization.hpp"

using namespace CitronECS;

EntitySnapshot::EntitySnapshot(StreamReader &reader, std::shared_ptr<Scene> parentScene, UUID parentId, bool restoreData) {
	this->parentId = parentId;
	this->parentScene = parentScene;
	if (restoreData) {
		restore(reader);
	} else {
		deserialize(reader);
	}
}


//TODO, make sure member bytes actully contains a duplicate of the data, not just a pointer to the source
EntitySnapshot::EntitySnapshot(Entity entity, std::shared_ptr<Scene> parentScene) : parentScene(parentScene) {
	this->entity = {
		.handle = entity.getHandle(),
	};
	BufferStreamWriter writer = BufferStreamWriter(buffer);
	for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
		if (metadata.has(parentScene->getRegistry(), entity)) {
			this->entity.componentMetadata[typeHash] = metadata;
		}
	}
	EntityBaseComponent &base = entity.getComponent<EntityBaseComponent>();
	for (UUID childID : base.children) {
		Entity childEntity = parentScene->getEntity(childID);
		children.push_back(EntitySnapshot(childEntity, parentScene));
	}

	serialize(writer);
}

void EntitySnapshot::serialize(StreamWriter &writer) {
	// write data for target entity
	size_t numChildComponents = entity.componentMetadata.size();
	writer.writeData(&numChildComponents, sizeof(numChildComponents));
	for (const auto &[typeHash, metadata] : entity.componentMetadata) {
		writer.writeData(&typeHash, sizeof(typeHash));
		void *component = metadata.get(parentScene->getRegistry(), entity.handle);
		for (const Member &member : metadata.members) {
			void *memberBytes = (char *)component + member.offset;
			member.serialize(writer, memberBytes);
		}
	}

	// write data for child entities
	size_t numChildren = children.size();
	writer.writeData(&numChildren, sizeof(numChildren));
	for (EntitySnapshot &child : children) {
		child.serialize(writer);
	}
}

void EntitySnapshot::deserialize(StreamReader &reader) {
	size_t numComponents;
	entt::entity targetEntity = parentScene->getRegistry().create();
	reader.readData(&numComponents, sizeof(numComponents));
	for (size_t j = 0; j < numComponents; j++) {
		uint32_t typeHash;
		reader.readData(&typeHash, sizeof(typeHash));

		CITRON_CORE_ASSERT(ECSRegistry::getComponentRegistry().contains(typeHash), "Component of type hash {} not found", typeHash);

		ComponentMetadata metadata = ECSRegistry::getComponentRegistry()[typeHash];
		metadata.add(parentScene->getRegistry(), targetEntity);
		void *component = metadata.get(parentScene->getRegistry(), targetEntity);
		for (Member &member : metadata.members) {
			void *memberBytes = (char *)component + member.offset;
			member.deserialize(reader, memberBytes);
		}
	}

	Entity target = parentScene->getEntity(targetEntity);
	EntityBaseComponent &base = target.getComponent<EntityBaseComponent>();
	base.uuid = UUID();
	parentScene->getEntityMap()[base.uuid] = targetEntity;
	entity.handle = targetEntity;
	for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
		if (metadata.has(parentScene->getRegistry(), targetEntity)) {
			entity.componentMetadata[typeHash] = metadata;
		}
	}
	base.parentId = parentId;
	base.children.clear();
	
	size_t numChildren;
	reader.readData(&numChildren, sizeof(numChildren));
	for (size_t i = 0; i < numChildren; i++) {
		children.push_back(EntitySnapshot(reader, parentScene, target.getComponent<EntityBaseComponent>().uuid));
		base.children.push_back(parentScene->getRegistry().get<EntityBaseComponent>(children.back().entity.handle).uuid);
	}
}

void EntitySnapshot::restore(StreamReader& reader) {
	size_t numComponents;
	entt::entity targetEntity = parentScene->getRegistry().create();
	reader.readData(&numComponents, sizeof(numComponents));
	for (size_t j = 0; j < numComponents; j++) {
		uint32_t typeHash;
		reader.readData(&typeHash, sizeof(typeHash));

		CITRON_CORE_ASSERT(ECSRegistry::getComponentRegistry().contains(typeHash), "Component of type hash {} not found", typeHash);

		ComponentMetadata metadata = ECSRegistry::getComponentRegistry()[typeHash];
		metadata.add(parentScene->getRegistry(), targetEntity);
		void *component = metadata.get(parentScene->getRegistry(), targetEntity);
		for (Member &member : metadata.members) {
			void *memberBytes = (char *)component + member.offset;
			member.deserialize(reader, memberBytes);
		}
	}

	Entity target = parentScene->getEntity(targetEntity);
	EntityBaseComponent &base = target.getComponent<EntityBaseComponent>();
	parentScene->getEntityMap()[base.uuid] = targetEntity;
	entity.handle = targetEntity;
	for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
		if (metadata.has(parentScene->getRegistry(), targetEntity)) {
			entity.componentMetadata[typeHash] = metadata;
		}
	}
	
	size_t numChildren;
	reader.readData(&numChildren, sizeof(numChildren));
	for (size_t i = 0; i < numChildren; i++) {
		children.push_back(EntitySnapshot(reader, parentScene, target.getComponent<EntityBaseComponent>().uuid, true));
	}
}

void EntitySnapshot::apply() {
	BufferStreamReader reader = BufferStreamReader(buffer.data(), buffer.size());
	restore(reader);
}
