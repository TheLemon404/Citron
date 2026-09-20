#pragma once

#include "ecs.hpp"
#include "registry.hpp"
#include "uuid.hpp"
#include <stack>
#include <memory>
class ICommand {
  public:
	virtual void execute() = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

class CommandManager {
	std::stack<std::unique_ptr<ICommand>> action_stack;
	std::stack<std::unique_ptr<ICommand>> undo_stack;

  public:
	void execute(std::unique_ptr<ICommand> command);
	void undo();
	void redo();
};

class CreateSystemCommand : public ICommand {
	CitronECS::SystemMetadata metadata;
	std::shared_ptr<CitronECS::Scene> scene;

  public:
	CreateSystemCommand(CitronECS::SystemMetadata metadata, std::shared_ptr<CitronECS::Scene> scene) : metadata(metadata), scene(scene) {}

	void execute() override;
	void undo() override;
	void redo() override;
};

class RemoveSystemCommand : public ICommand {
	std::shared_ptr<CitronECS::System> system;
	std::shared_ptr<CitronECS::Scene> scene;

  public:
	RemoveSystemCommand(std::shared_ptr<CitronECS::System> system, std::shared_ptr<CitronECS::Scene> scene) : system(system), scene(scene) {}

	void execute() override;
	void undo() override;
	void redo() override;
};

class CreateEntityCommand : public ICommand {
	UUID parentId;
	std::shared_ptr<CitronECS::Scene> scene;
	UUID newEntityId = UUID::nullID;

  public:
	CreateEntityCommand(UUID parentId, std::shared_ptr<CitronECS::Scene> scene) : parentId(parentId),
																				  scene(scene) {}
	void execute() override;
	void undo() override;
	void redo() override;
};

class DeleteEntitiesCommand : public ICommand {
  public:
	DeleteEntitiesCommand();

	void execute() override;
	void undo() override;
	void redo() override;
};

class AddComponentCommand : public ICommand {
  public:
	void execute() override;
	void undo() override;
	void redo() override;
};

class RemoveComponentCommand : public ICommand {
  public:
	void execute() override;
	void undo() override;
	void redo() override;
};

class EditComponentCommand : public ICommand {
  public:
	void execute() override;
	void undo() override;
	void redo() override;
};
