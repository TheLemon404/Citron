#pragma once

#include "ecs.hpp"
#include "entt/entity/fwd.hpp"
#include "registry.hpp"
#include "uuid.hpp"
#include <stack>
#include <memory>

using SceneSelectionItem = std::variant<entt::entity, std::shared_ptr<CitronECS::System>>;

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
	UUID primaryEntityId;
	std::set<SceneSelectionItem> &secondaryItems;
	std::unordered_set<UUID> deletedEntities;
	std::shared_ptr<CitronECS::Scene> scene;

  public:
	DeleteEntitiesCommand(UUID primaryEntityId, std::set<SceneSelectionItem> &secondaryItems, std::shared_ptr<CitronECS::Scene> scene) : primaryEntityId(primaryEntityId), secondaryItems(secondaryItems), scene(scene) {}

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
