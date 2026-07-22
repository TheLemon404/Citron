#include "panel.hpp"

#include "IconsFontAwesome5.h"
#include "ecs.hpp"
#include "editor.hpp"
#include <cfloat>
#include <event.hpp>
#include <float.h>
#include <io.hpp>
#include <logger.hpp>

#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "keyboard.hpp"
#include "spdlog/common.h"
#include <IconsFontAwesome5.h>
#include <IconsFontAwesome6.h>
#include <imgui_stdlib.h>
#include <memory>

using namespace CitronCore;
using namespace CitronECS;

bool CustomCollapsingHeader(const char *label, bool *p_open,
							const char *icon_open = ICON_FA_SQUARE_MINUS,
							const char *icon_closed = ICON_FA_SQUARE_PLUS) {
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;

	// Align button text to the left
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

	// Format full-width label with your custom trailing/leading icons
	char buf[128];
	snprintf(buf, sizeof(buf), "%s  %s", (*p_open ? icon_open : icon_closed),
			 label);

	// Draw full-width style frame
	if (ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f))) {
		*p_open = !*p_open;
	}

	ImGui::PopStyleVar();
	return *p_open;
}

void AssetPanel::onAttach() {
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	currentDirectory = context.projectRootFolderPath;
	refreshDirectoryListings();
}
void AssetPanel::onDetach() {}
void AssetPanel::onUpdate() {}
void AssetPanel::onDraw() {
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();

	ImGui::Begin("Assets");
	ImGui::BeginGroup();
	if (ImGui::Button(ICON_FA_ARROWS_ROTATE)) {
		refreshDirectoryListings();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ARROW_UP)) {
		if (!currentDirectory.empty()) {
			currentDirectory =
				CitronIO::IO::getParentDirectory(currentDirectory);
			refreshDirectoryListings();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_PLUS))
		zoomLevel += 50;
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_MINUS))
		zoomLevel -= 50;
	zoomLevel = std::max(100, zoomLevel);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::InputText("##currentDirectory", &currentDirectory);

	ImGui::EndGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(20.0f, 20.0f));
	ImGui::BeginChild("AssetList");

	bool createFolder = false;

	if (ImGui::BeginPopupContextWindow(
			"AssetBrowserPopup", ImGuiPopupFlags_NoOpenOverExistingPopup)) {
		if (ImGui::MenuItem("Create Folder")) {
			createFolder = true;
		}

		ImGui::EndPopup();
	}

	if (createFolder) {
		ImGui::OpenPopup("CreateFolderPopup");
	}

	static std::string folderName;
	if (ImGui::BeginPopup("CreateFolderPopup")) {
		if (ImGui::InputTextWithHint("Create Folder", "Folder Name",
									 &folderName,
									 ImGuiInputTextFlags_EnterReturnsTrue)) {
			ImGui::InputTextWithHint("Directory Name", "Directory Name",
									 &folderName);
			CitronIO::IO::createDirectory(currentDirectory + "\\" + folderName);
			CITRON_CLIENT_INFO("Created new directory {}",
							   currentDirectory + "\\" + folderName);
			pendingRefreshDirectory = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Dummy(ImVec2(0.0f, 4.0f));
	int i = 0;
	for (const auto &entry : directoryListings) {
		ImGui::Columns(ImGui::GetCurrentWindow()->Size.x / zoomLevel, nullptr,
					   false);
		ImGui::PushID(i++);

		if (entry.isDirectory) {
			ImGui::SetWindowFontScale(5.0f * zoomLevel / 150.0f);
			if (ImGui::Button(ICON_FA_FOLDER,
							  ImVec2(zoomLevel * 0.9f, zoomLevel))) {
				currentDirectory = entry.path;
				pendingRefreshDirectory = true;
			}

			if (ImGui::IsItemHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup("FolderPopup");
			}

			bool renameFolder = false;

			if (ImGui::BeginPopup("FolderPopup")) {
				if (ImGui::MenuItem("Rename")) {
					renameFolder = true;
				} else if (ImGui::MenuItem("Delete")) {
					CitronIO::IO::deleteDirectory(entry.path);
					ImGui::CloseCurrentPopup();
					pendingRefreshDirectory = true;
				}

				ImGui::EndPopup();
			}

			if (renameFolder) {
				ImGui::OpenPopup("FolderRenamePopup");
			}

			static std::string folderName;
			if (ImGui::BeginPopup("FolderRenamePopup")) {
				if (ImGui::InputTextWithHint(
						"Rename Folder", "Folder Name", &folderName,
						ImGuiInputTextFlags_EnterReturnsTrue)) {
					std::string newPath =
						entry.path.substr(0,
										  entry.path.find_last_of('\\') + 1) +
						folderName;
					CitronIO::IO::renameDirectory(entry.path, newPath);
					ImGui::CloseCurrentPopup();
					pendingRefreshDirectory = true;

					CITRON_CORE_INFO("Renamed folder {} to {}", entry.path,
									 newPath);
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text(entry.name.c_str());
		} else {
			ImGui::SetWindowFontScale(6.0f * zoomLevel / 150.0f);
			ImGui::Button(ICON_FA_FILE, ImVec2(zoomLevel * 0.9f, zoomLevel));

			if (ImGui::IsItemHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup("FilePopup");
			}

			bool renameFile = false;

			if (ImGui::BeginPopup("FilePopup")) {
				if (ImGui::MenuItem("Rename")) {
					renameFile = true;
				} else if (ImGui::MenuItem("Delete")) {
					if (context.currentlyEditedSceneAssetPath == entry.path) {
						context.currentlyEditedSceneAssetPath = "";
					}

					CitronIO::IO::deleteDirectory(entry.path);
					ImGui::CloseCurrentPopup();
					pendingRefreshDirectory = true;
				}

				ImGui::EndPopup();
			}

			if (renameFile) {
				ImGui::OpenPopup("FileRenamePopup");
			}

			static std::string fileName;
			if (ImGui::BeginPopup("FileRenamePopup")) {
				if (ImGui::InputTextWithHint(
						"Rename Folder", "Folder Name", &fileName,
						ImGuiInputTextFlags_EnterReturnsTrue)) {
					std::string fileExtension =
						entry.path.substr(entry.path.find_last_of('.'));
					std::string newPath =
						entry.path.substr(0,
										  entry.path.find_last_of('\\') + 1) +
						fileName + fileExtension;
					if (context.currentlyEditedSceneAssetPath == entry.path) {
						context.currentlyEditedSceneAssetPath = newPath;
						context.getCurrentScene()->rename(fileName);
					}

					CitronIO::IO::renameDirectory(entry.path, newPath);
					ImGui::CloseCurrentPopup();

					CITRON_CORE_INFO("Renamed file {} to {}", entry.path,
									 newPath);

					pendingRefreshDirectory = true;
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text(entry.name.c_str());
		}

		ImGui::SetWindowFontScale(1.0f);

		ImGui::PopID();
		ImGui::NextColumn();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();

	ImGui::End();

	if (pendingRefreshDirectory) {
		refreshDirectoryListings();
		pendingRefreshDirectory = false;
	}
}
void AssetPanel::onEvent(Event &e) {
	if (e.isInCategory(CitronCore::EventCategoryInput)) {
		if (e.getEventType() == EventType::KeyJustPressed) {
			KeyJustPressedEvent &event = static_cast<KeyJustPressedEvent &>(e);
			if (event.getKeycode() == SDLK_S && event.getMods() & SDLK_LCTRL) {
				pendingRefreshDirectory = true;
			}
		}
	}
}

void AssetPanel::refreshDirectoryListings() {
	CITRON_CLIENT_INFO("Refreshed directory listings for: {}",
					   currentDirectory);
	directoryListings.clear();
	for (std::string &entry :
		 CitronIO::IO::getDirectoryEntries(currentDirectory)) {
		AssetCard card = {};
		card.path = entry;
		card.name = entry.substr(entry.find_last_of("\\") + 1);
		card.isDirectory = CitronIO::IO::isDirectory(entry);
		directoryListings.push_back(card);
	}
}

void ConsolePanel::onAttach() {}
void ConsolePanel::onDetach() {}
void ConsolePanel::onUpdate() {}
void ConsolePanel::onDraw() {
	ImGui::Begin("Log");
	if (ImGui::Button("Clear")) {
		Editor::get().getLogSink()->entries.clear();
	}

	if (ImGui::BeginTable("LogTable", 4,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 1.0f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								75.0f);
		ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed,
								100.0f);
		ImGui::TableSetupColumn("Message");
		ImGui::TableSetupScrollFreeze(3, 1);
		ImGui::TableHeadersRow();

		for (LogEntry &logEntry : Editor::get().getLogSink()->entries) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImVec4 color;
			switch (logEntry.logLevel) {
			case spdlog::level::debug:
				color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
				break;
			case spdlog::level::info:
				color = ImVec4(0.27843137254f, 0.44705882352f, 0.70196078431f,
							   1.0f);
				break;
			case spdlog::level::warn:
				color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
				break;
			case spdlog::level::err:
				color = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);
				break;
			case spdlog::level::critical:
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				break;
			default:
				color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
				break;
			}
			ImU32 cell_color = ImGui::ColorConvertFloat4ToU32(color); // Red
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_color);
			ImGui::TableNextColumn();
			ImGui::Text("%s", logEntry.type.c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%u", logEntry.timestamp);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(logEntry.message.c_str());
		}
		ImGui::EndTable();
	}
	ImGui::End();
}
void ConsolePanel::onEvent(Event &e) {}

void OutlinerPanel::onAttach() {}
void OutlinerPanel::onDetach() {}
void OutlinerPanel::onUpdate() {
	std::shared_ptr<Scene> currentEditedScene = Editor::get()
													.getLayerStack()
													.getLayer<EditorLayer>()
													->getEditorContext()
													.getCurrentScene();
	if (pendingCreateEntity) {
		pendingCreateEntity = false;
		UUID newEntity = currentEditedScene->createEntity();
		if (pendingCreateEntityParent != UUID::nullID) {
			currentEditedScene->reparentEntity(
				currentEditedScene->getEntity(newEntity),
				currentEditedScene->getEntity(pendingCreateEntityParent));
			pendingCreateEntityParent = UUID::nullID;
		}
	}
	if (pendingDeleteEntity != UUID::nullID) {
		currentEditedScene->deleteEntity(pendingDeleteEntity);
		pendingDeleteEntity = UUID::nullID;
	}
}

void OutlinerPanel::showEntityChildTree(entt::entity entity,
										std::shared_ptr<Scene> &scene) {
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	CitronECS::EntityBase &entityBase =
		scene->getRegistry().get<CitronECS::EntityBase>(entity);

	ImGui::PushID(entityBase.uuid);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	bool node1_open = ImGui::TreeNode(entityBase.name.c_str());
	if (ImGui::IsItemClicked()) {
		context.setCurrentSelectedEntity(entity);
	}

	if (ImGui::BeginPopupContextItem("EntityContextPopup")) {
		if (ImGui::MenuItem("Delete Entity")) {
			pendingDeleteEntity = entityBase.uuid;
		}
		if (ImGui::MenuItem("Create Entity")) {
			context.setCurrentSelectedEntity(entt::null);
			pendingCreateEntity = true;
			pendingCreateEntityParent = entityBase.uuid;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::TableSetColumnIndex(1);
	ImGui::Text("Entity");

	if (node1_open) {
		for (UUID childID : entityBase.children) {
			showEntityChildTree(scene->getEntity(childID), scene);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void OutlinerPanel::onDraw() {
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	std::shared_ptr<Scene> currentEditedScene = context.getCurrentScene();

	ImGui::Begin("Outliner");

	std::string entitySearchResult;
	ImGui::InputTextWithHint(ICON_FA_MAGNIFYING_GLASS, "Search by entity name",
							 &entitySearchResult);
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_PLUS_CIRCLE)) {
		ImGui::OpenPopup("ActionsPopup");
	}
	if (ImGui::BeginPopup("ActionsPopup")) {
		if (ImGui::MenuItem("Add System")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Create Entity")) {
			if (currentEditedScene) {
				context.setCurrentSelectedEntity(entt::null);
				pendingCreateEntity = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginTable("LogTable", 2,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,
								75.0f);
		ImGui::TableHeadersRow();

		int i = 0;
		if (currentEditedScene) {
			for (std::shared_ptr<System> &system :
				 currentEditedScene->getSystems()) {
				ImGui::PushID(i++);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::Selectable(system->getName().c_str(), false,
								  ImGuiSelectableFlags_SpanAllColumns);
				ImGui::TableNextColumn();
				ImGui::Text("System");
				ImGui::PopID();
			}
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("--");
		ImGui::TableNextColumn();
		ImGui::Text("--");

		if (currentEditedScene) {
			const auto &view =
				currentEditedScene->getRegistry().view<EntityBase>();
			for (const entt::entity &entity : view) {
				auto &entityBase = view.get<EntityBase>(entity);
				if (entityBase.parentId == 0) {
					showEntityChildTree(entity, currentEditedScene);
				}
			}

			if (ImGui::BeginPopupContextWindow(
					"SceneContextPopup",
					ImGuiPopupFlags_NoOpenOverExistingPopup)) {
				if (ImGui::MenuItem("Add System")) {
				}
				if (ImGui::MenuItem("Create Entity")) {
					currentEditedScene->createEntity();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
void OutlinerPanel::onEvent(Event &e) {}

void InspectorPanel::onAttach() {}
void InspectorPanel::onDetach() {}
void InspectorPanel::onUpdate() {}

void InspectorPanel::onDraw() {
	ImGui::Begin("Inspector");
	EditorContext &context = Editor::get()
								 .getLayerStack()
								 .getLayer<EditorLayer>()
								 ->getEditorContext();
	auto &registry = context.getCurrentScene()->getRegistry();
	const entt::entity selectedEntity = context.getCurrentSelectedEntity();
	if (selectedEntity != entt::null && registry.valid(selectedEntity)) {
		EntityBase &entityBase = registry.get<EntityBase>(selectedEntity);

		static bool selection = true;
		if (CustomCollapsingHeader("Entity Base", &selection)) {
			float width = ImGui::GetContentRegionAvail().x;

			ImGui::InputText("Name", &entityBase.name);
			ImGui::Text("ID: %u", (unsigned int)entityBase.uuid);
		}

		if (ImGui::Button("Add Component", ImVec2(-FLT_MIN, 0.0f))) {
			ImGui::OpenPopup("ActionsPopup");
		}
		if (ImGui::BeginPopup("ActionsPopup")) {
			std::string componentSearchResult;
			ImGui::InputTextWithHint("##ComponentSearch",
									 "Enter Component Class Name",
									 &componentSearchResult);
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}
void InspectorPanel::onEvent(Event &e) {}
