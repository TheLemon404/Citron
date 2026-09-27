#include "snapshot.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "entt/entity/fwd.hpp"
#include "serialization.hpp"

using namespace CitronECS;

EntitySnapshot::EntitySnapshot(StreamReader &reader, std::shared_ptr<Scene> parentScene) {
	this->parentScene = parentScene;
	deserialize(reader);
}

EntitySnapshot::EntitySnapshot(Entity entity, std::shared_ptr<Scene> parentScene) : parentScene(parentScene) {
	this->entity = {
		.handle = entity.getHandle(),
	};
	for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
		if (metadata.has(parentScene->getRegistry(), entity)) {
			this->entity.componentMetadata[typeHash] = metadata;
			for (const Member &member : metadata.members) {
				void *component = metadata.get(parentScene->getRegistry(), entity);
				this->entity.memberBytes[member.fieldName] = component;
			}
		}
	}
	
	EntityBaseComponent &base = entity.getComponent<EntityBaseComponent>();
	for (UUID childID : base.children) {
		Entity childEntity = parentScene->getEntity(childID);
		children.push_back(EntitySnapshot(childEntity, parentScene));
	}
}

void EntitySnapshot::serialize(StreamWriter &writer) {
	// write data for target entity
	size_t numChildComponents = entity.componentMetadata.size();
	writer.writeData(&numChildComponents, sizeof(numChildComponents));
	for (const auto &[typeHash, metadata] : entity.componentMetadata) {
		writer.writeData(&typeHash, sizeof(typeHash));
		for (const Member &member : metadata.members) {
			void *component = entity.memberBytes.at(member.fieldName);
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
		for (Member &member : metadata.members) {
			void *component = metadata.get(parentScene->getRegistry(), targetEntity);
			void *memberBytes = (char *)component + member.offset;
			member.deserialize(reader, memberBytes);
		}
	}

	size_t numChildren;
	reader.readData(&numChildren, sizeof(numChildren));
	for (size_t i = 0; i < numChildren; i++) {
		children.push_back(EntitySnapshot(reader, parentScene));
	}

	parentScene->randomizeEntityUUID(parentScene->getEntity(targetEntity));
	entity.handle = targetEntity;
	for (const auto &[typeHash, metadata] : ECSRegistry::getComponentRegistry()) {
		if (metadata.has(parentScene->getRegistry(), targetEntity)) {
			entity.componentMetadata[typeHash] = metadata;
			for (const Member &member : metadata.members) {
				void *component = metadata.get(parentScene->getRegistry(), targetEntity);
				entity.memberBytes[member.fieldName] = component;
			}
		}
	}
}

void EntitySnapshot::apply() {
	Entity instantiatedRootEntity = parentScene->createEntity();
	for (const auto &[hashCode, metadata] : entity.componentMetadata) {
		metadata.add(parentScene->getRegistry(), instantiatedRootEntity);
		for (const Member &member : metadata.members) {
			void *savedComponent = entity.memberBytes.at(member.fieldName);
			void *memberBytes = (char *)savedComponent + member.offset;
			void *component = metadata.get(parentScene->getRegistry(), instantiatedRootEntity);
			MemoryStreamWriter writer = MemoryStreamWriter(component);
			member.serialize(writer, savedComponent);
		}
	}
	for (EntitySnapshot &childSnapshot : children) {
		childSnapshot.apply();
	}
}
