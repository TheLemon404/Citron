#include "command.hpp"
#include "component.hpp"
#include "ecs.hpp"
#include "logger.hpp"

void CommandManager::execute(std::unique_ptr<ICommand> command) {
	command->execute();
	action_stack.push(std::move(command));
	undo_stack = std::stack<std::unique_ptr<ICommand>>();
}

void CommandManager::undo() {
	if (!action_stack.empty()) {
		auto command = std::move(action_stack.top());
		action_stack.pop();
		command->undo();
		undo_stack.push(std::move(command));
	}
}

void CommandManager::redo() {
	if (!undo_stack.empty()) {
		auto command = std::move(undo_stack.top());
		undo_stack.pop();
		command->redo();
		action_stack.push(std::move(command));
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
