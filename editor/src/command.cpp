#include "command.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "logger.hpp"
#include "snapshot.hpp"

constexpr uint8_t MAX_UNDO_STACK_SIZE = 50;

void CommandManager::execute(std::unique_ptr<ICommand> command) {
	command->execute();
	action_stack.push_front(std::move(command));
	undo_stack = std::stack<std::unique_ptr<ICommand>>();
	if (action_stack.size() > MAX_UNDO_STACK_SIZE) {
		action_stack.pop_back();
	}
}

void CommandManager::undo() {
	if (!action_stack.empty()) {
		auto command = std::move(action_stack.front());
		action_stack.pop_front();
		command->undo();
		undo_stack.push(std::move(command));
	}
}

void CommandManager::redo() {
	if (!undo_stack.empty()) {
		auto command = std::move(undo_stack.top());
		undo_stack.pop();
		command->redo();
		action_stack.push_front(std::move(command));
	}
}

void CreateSystemCommand::execute() {
	metadata.add(scene);
}

void CreateSystemCommand::undo() {
	metadata.remove(scene);
}

void CreateSystemCommand::redo() {
	metadata.add(scene);
}

void RemoveSystemCommand::execute() {
	scene->removeSystem(system);
}

void RemoveSystemCommand::undo() {
	scene->addSystem(system);
}

void RemoveSystemCommand::redo() {
	scene->removeSystem(system);
}

void CreateEntityCommand::execute() {
	CitronECS::Entity newEntity = scene->createEntity();
	newEntityId = newEntity.getComponent<CitronECS::EntityBaseComponent>().uuid;
	if (parentId != UUID::nullID) {
		scene->reparentEntity(newEntity,
							  scene->getEntity(parentId));
	}
}

void CreateEntityCommand::undo() {
	if (newEntityId != UUID::nullID) {
		scene->deleteEntity(scene->getEntity(newEntityId));
	}
}

void CreateEntityCommand::redo() {
	if (newEntityId != UUID::nullID) {
		CitronECS::Entity newEntity = scene->createEntity(newEntityId);
		if (parentId != UUID::nullID) {
			scene->reparentEntity(newEntity,
								  scene->getEntity(parentId));
		}
	}
}

void DeleteEntitiesCommand::execute() {
	CitronECS::Entity primaryEntityToDelete = scene->getEntity(primaryEntityId);
	snapshots.push_back(CitronECS::EntitySnapshot(primaryEntityToDelete, scene));
	for (const std::variant<entt::entity, std::shared_ptr<CitronECS::System>> entity : secondaryItems) {
		CitronECS::Entity entityToDelete = scene->getEntity(std::get<entt::entity>(entity));
		if (entityToDelete.getComponent<CitronECS::EntityBaseComponent>().parentId == UUID::nullID) {
			snapshots.push_back(CitronECS::EntitySnapshot(entityToDelete, scene));
		}
	}

	for (const std::variant<entt::entity, std::shared_ptr<CitronECS::System>> entity : secondaryItems) {
		CitronECS::Entity entityToDelete = scene->getEntity(std::get<entt::entity>(entity));
		scene->deleteEntity(entityToDelete);
	}
	if (scene->hasEntity(primaryEntityId)) {
		scene->deleteEntity(scene->getEntity(primaryEntityId));
	}
}

void DeleteEntitiesCommand::undo() {
	for (CitronECS::EntitySnapshot &snapshot : snapshots) {
		snapshot.apply();
	}
}

void DeleteEntitiesCommand::redo() {
	for (const UUID uuid : deletedEntities) {
		scene->deleteEntity(scene->getEntity(uuid));
	}
	if (scene->hasEntity(primaryEntityId)) {
		scene->deleteEntity(scene->getEntity(primaryEntityId));
	}
}

void EditComponentCommand::execute() {
}

void EditComponentCommand::undo() {
}

void EditComponentCommand::redo() {
	
}
