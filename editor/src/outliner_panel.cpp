#include "outliner_panel.hpp"
#include "editor.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include "logger.hpp"

#include <ecs.hpp>
#include <component.hpp>
#include <input.hpp>
#include <imgui_stdlib.h>

void OutlinerPanel::onAttach() {}
void OutlinerPanel::onDetach() {}
void OutlinerPanel::onUpdate() {
	std::shared_ptr<Scene> currentEditedScene =
		appContext.sceneManager.getActiveScene();
	if (pendingCreateEntity) {
		pendingCreateEntity = false;
		Entity newEntity = currentEditedScene->createEntity();
		if (pendingCreateEntityParent != UUID::nullID) {
			currentEditedScene->reparentEntity(newEntity,
											   currentEditedScene->getEntity(pendingCreateEntityParent));
			pendingCreateEntityParent = UUID::nullID;
		}
	}

	if (pendingDeleteEntity != UUID::nullID) {
		for (const std::variant<entt::entity, std::shared_ptr<System>> entity : Editor::get().getEditorContext().getSecondarySelectedItems()) {
			currentEditedScene->deleteEntity(currentEditedScene->getEntity(std::get<entt::entity>(entity)));
		}
		if (currentEditedScene->hasEntity(pendingDeleteEntity)) {
			currentEditedScene->deleteEntity(currentEditedScene->getEntity(pendingDeleteEntity));
		}
		pendingDeleteEntity = UUID::nullID;
		Editor::get().getEditorContext().setCurrentlySelectedItem(nullptr);
	}
	if (pendingDeleteSystem != nullptr) {
		currentEditedScene->removeSystem(pendingDeleteSystem);
		pendingDeleteSystem = nullptr;
		Editor::get().getEditorContext().setCurrentlySelectedItem(nullptr);
	}
}

void OutlinerPanel::showEntityChildTree(entt::entity entity,
										std::shared_ptr<Scene> &scene) {
	EditorContext &context = Editor::get().getEditorContext();
	CitronECS::EntityBaseComponent &entityBase =
		scene->getRegistry().get<CitronECS::EntityBaseComponent>(entity);

	EditorIcons &icons = Editor::get().getLayer<GuiLayer>()->editorIcons;
	WGPUTextureView iconView = icons.getTextureView();

	ImGui::PushID(entityBase.uuid);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_FramePadding;
	bool selected = context.getCurrentlySelectedItem().index() == 0 && std::get<entt::entity>(context.getCurrentlySelectedItem()) == entity;
	bool secondarySelected = context.getSecondarySelectedItems().contains(entity);
	bool isLeaf = scene->getRegistry().get<CitronECS::EntityBaseComponent>(entity).children.empty();
	if (isLeaf) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (secondarySelected) {
		ImGui::PushStyleColor(ImGuiCol_Header, themeSecondarySelectedColor);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, themeSecondarySelectedColor);
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (selected) {
		ImGui::PushStyleColor(ImGuiCol_Header, themeSecondaryColor);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, themeSecondaryColor);
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	bool node1_open = ImGui::TreeNodeEx(entityBase.name.c_str(),
										flags);

	// icon
	ImVec2 rectMin = ImGui::GetItemRectMin();
	ImVec2 rectMax = ImGui::GetItemRectMax();
	float size = rectMax.y - rectMin.y;
	ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
	ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
	Icon icon = isLeaf ? icons.getIcon("Entity") : icons.getIcon("ParentEntity");
	ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icon.uv.Min, icon.uv.Max);

	ImGui::PopStyleVar();
	if (selected) {
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	if (secondarySelected) {
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("ENTITY_TREE_REORDER",
								  (uint32_t *)&entityBase.uuid,
								  sizeof(uint32_t));
		ImGui::Text("Reparenting Entity: %s", entityBase.name.c_str());
		ImGui::EndDragDropSource();
	} else if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload *payload =
				ImGui::AcceptDragDropPayload("ENTITY_TREE_REORDER")) {
			uint32_t *childEntityUUID = (uint32_t *)payload->Data;
			UUID newChildUUID = *childEntityUUID;
			std::shared_ptr<Scene> currentScene = appContext.sceneManager.getActiveScene();
			currentScene->reparentEntity(
				currentScene->getEntity(newChildUUID),
				currentScene->getEntity(entityBase.uuid));
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::IsItemClicked()) {
		CitronInput::InputLayer *inputLayer = Editor::get().getLayer<CitronInput::InputLayer>();

		CITRON_CLIENT_INFO("i {}", scene->getEntityIndex(entity));

		if (inputLayer->isJustReleased(SDLK_LSHIFT)) {
			shiftSelectStartEntity = entt::null;
		}

		if (inputLayer->isPressed(SDLK_LSHIFT) && context.getCurrentlySelectedItem().index() == 0) {
			shiftSelectStartEntity = std::get<entt::entity>(context.getCurrentlySelectedItem());

			uint32_t i = scene->getEntityIndex(shiftSelectStartEntity);
			uint32_t j = scene->getEntityIndex(entity);

			CITRON_CLIENT_INFO("i {} j {}", i, j);

			uint32_t start = i < j ? i : j;
			uint32_t end = i < j ? j : i;

			for (Entity e : scene->getRootEntities()) {
				if (start <= scene->getEntityIndex(e) && scene->getEntityIndex(e) <= end) {
					context.addSecondarySelectedItem(e);
				}
			}

			context.setCurrentlySelectedItem(entity, true);
		} else {
			context.setCurrentlySelectedItem(entity, inputLayer->isPressed(SDLK_LCTRL) ? true : false);
		}
	}

	if (ImGui::BeginPopupContextItem("EntityContextPopup")) {
		if (ImGui::MenuItem("Delete Entity")) {
			pendingDeleteEntity = entityBase.uuid;
		}
		if (ImGui::MenuItem("Create Entity")) {
			context.setCurrentlySelectedItem(entt::null);
			pendingCreateEntity = true;
			pendingCreateEntityParent = entityBase.uuid;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (node1_open) {
		for (UUID childID : entityBase.children) {
			showEntityChildTree(scene->getEntity(childID), scene);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void OutlinerPanel::onDraw() {
	EditorContext &context = Editor::get().getEditorContext();
	std::shared_ptr<Scene> currentEditedScene = appContext.sceneManager.getActiveScene();

	ImGui::Begin("Outliner");
	std::string entitySearchResult;
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##EntitySearch", "Search by entity name",
							 &entitySearchResult);

	bool pendingAddSystem = false;

	if (ImGui::BeginTable("##SystemsTable", 1)) {
		ImGui::TableSetupColumn("  Systems");
		ImGui::TableHeadersRow();
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

		if (ImGui::BeginPopupContextItem(
				"SceneContextPopup",
				ImGuiPopupFlags_NoOpenOverExistingPopup)) {
			if (ImGui::MenuItem("Add System")) {
				pendingAddSystem = true;
			}
			ImGui::EndPopup();
		}

		EditorIcons &icons = Editor::get().getLayer<GuiLayer>()->editorIcons;
		WGPUTextureView iconView = icons.getTextureView();

		for (auto &[id, system] : currentEditedScene->getSystems()) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			bool selected = context.getCurrentlySelectedItem().index() == 1 && std::get<std::shared_ptr<System>>(context.getCurrentlySelectedItem()) == system;
			if (selected) {
				ImGui::PushStyleColor(ImGuiCol_Header, themeSecondaryColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, themeSecondaryColor);

				flags |= ImGuiTreeNodeFlags_Selected;
			}
			ImGui::PushID(id);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0, 4.0));
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TreeNodeEx(system->getName().c_str(), flags);

			if (ImGui::BeginPopupContextItem(
					"SceneContextPopup",
					ImGuiPopupFlags_NoOpenOverExistingPopup)) {
				if (ImGui::MenuItem("Delete System") && context.getCurrentlySelectedItem().index() == 1) {
					pendingDeleteSystem = system;
				}
				if (ImGui::MenuItem("Add System")) {
					pendingAddSystem = true;
				}
				ImGui::EndPopup();
			}

			// icon
			ImVec2 rectMin = ImGui::GetItemRectMin();
			ImVec2 rectMax = ImGui::GetItemRectMax();
			float size = rectMax.y - rectMin.y;
			ImVec2 iconMin = ImVec2(rectMin.x, rectMin.y);
			ImVec2 iconMax = ImVec2(iconMin.x + size, iconMin.y + size);
			ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, iconMin, iconMax, icons.getIcon("System").uv.Min, icons.getIcon("System").uv.Max);

			if (ImGui::IsItemClicked()) {
				context.setCurrentlySelectedItem(system);
			}
			ImGui::PopStyleVar();
			ImGui::PopID();
			if (selected) {
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}

		ImGui::PopStyleVar();

		ImGui::EndTable();
	}

	if (ImGui::BeginTable("##EntityTable", 1,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("  Entities");
		ImGui::TableHeadersRow();

		if (ImGui::BeginPopupContextItem(
				"SceneContextPopup",
				ImGuiPopupFlags_NoOpenOverExistingPopup)) {
			if (ImGui::MenuItem("Create Entity")) {
				currentEditedScene->createEntity();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload =
					ImGui::AcceptDragDropPayload("ENTITY_TREE_REORDER")) {
				uint32_t *childEntityUUID = (uint32_t *)payload->Data;
				UUID newChildUUID = *childEntityUUID;
				std::shared_ptr<Scene> currentScene = appContext.sceneManager.getActiveScene();
				currentScene->reparentEntityToRoot(
					currentScene->getEntity(newChildUUID));
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

		if (currentEditedScene) {
			for (Entity entity : currentEditedScene->getRootEntities()) {
				showEntityChildTree(entity, currentEditedScene);
			}
		}

		ImGui::PopStyleVar();

		ImGui::EndTable();
	}

	if (pendingAddSystem) {
		ImGui::OpenPopup("SystemsPopup");
		pendingAddSystem = false;
	}

	if (currentEditedScene && ImGui::BeginPopup("SystemsPopup")) {
		std::string systemSearchResult;
		ImGui::InputTextWithHint("##SystemSearch",
								 "Enter System Class Name",
								 &systemSearchResult, ImGuiInputTextFlags_EnterReturnsTrue);
		for (const auto &[id, system] : ECSRegistry::getSystemRegistry()) {
			std::string searchLower = systemSearchResult;
			std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

			if (system.name.starts_with(systemSearchResult)) {
				if (ImGui::Selectable(system.name.c_str())) {
					systemSearchResult = system.name;
					system.add(currentEditedScene);
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void OutlinerPanel::onEvent(Event &e) {
	if (e.isInCategory(CitronCore::EventCategoryKeyboard)) {
		if (e.getEventType() == EventType::KeyPressed) {
			KeyPressedEvent &keyEvent = static_cast<KeyPressedEvent &>(e);
			if (keyEvent.getKeycode() == SDLK_DELETE) {
				if (Editor::get().getEditorContext().getCurrentlySelectedItem().index() == 0) {
					entt::entity e = std::get<entt::entity>(Editor::get().getEditorContext().getCurrentlySelectedItem());
					std::shared_ptr<Scene> currentEditedScene = appContext.sceneManager.getActiveScene();
					if (e != entt::null && currentEditedScene) {
						pendingDeleteEntity = currentEditedScene->getEntity(e).getComponent<EntityBaseComponent>().uuid;
					}
				}
			}
		}
	}
}
