#include "panel.hpp"

#include "IconsFontAwesome5.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_mouse.h"
#include "assets.hpp"
#include "editor.hpp"
#include <cfloat>
#include <component.hpp>
#include <concepts>
#include <cstdint>
#include <ecs.hpp>
#include <event.hpp>
#include <filesystem>
#include <float.h>
#include <io.hpp>
#include <logger.hpp>

#include "entt/entity/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "keyboard.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "spdlog/common.h"
#include <IconsFontAwesome5.h>
#include <IconsFontAwesome6.h>
#include <imgui_stdlib.h>
#include <memory>

void AssetPanel::onAttach() {
	EditorContext &context = Editor::get().getEditorContext();
	currentDirectory = context.projectFilePath.parent_path();
	refreshDirectoryListings();

	folderIconTexture = Texture::loadFromFile(std::filesystem::path(CITRON_PROGRAM_FOLDER) / "EngineResources/Textures/citron_folder.png", appContext.renderer.getContext().device);
}

void AssetPanel::onDetach() {}
void AssetPanel::onUpdate() {}
void AssetPanel::onDraw() {
	EditorContext &context = Editor::get().getEditorContext();

	ImGui::Begin("Assets");
	ImGui::BeginGroup();
	if (ImGui::Button(ICON_FA_ARROWS_ROTATE)) {
		refreshDirectoryListings();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ARROW_UP)) {
		if (!currentDirectory.empty() &&
			currentDirectory != context.projectFilePath.parent_path()) {
			currentDirectory = currentDirectory.parent_path();
			refreshDirectoryListings();
		}
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload *payload =
				ImGui::AcceptDragDropPayload("ASSET_FILE_TRANSFER")) {
			std::string srcPath((const char *)payload->Data, payload->DataSize);
			if (srcPath != context.currentlyEditedSceneAssetPath) {
				CitronIO::IO::moveFileOrFolder(srcPath,
											   currentDirectory.parent_path());
				appContext.assetManager.moveAsset(srcPath, currentDirectory.parent_path());
				pendingRefreshDirectory = true;
			} else {
				CITRON_CLIENT_ERROR(
					"Cannot move the currently edited scene asset");
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_PLUS))
		zoomLevel += 25;
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_MINUS))
		zoomLevel -= 25;
	zoomLevel = std::max(100, zoomLevel);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	std::string directoryTempString = currentDirectory.string();
	ImGui::InputText("##currentDirectory", &directoryTempString,
					 ImGuiInputTextFlags_ReadOnly);

	ImGui::EndGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(20.0f, 20.0f));
	ImGui::BeginChild("AssetList");

	WGPUTextureView view = folderIconTexture->getTextureView();
	if (ImGui::BeginTable("##AssetBrowserTable", std::max((int)(ImGui::GetCurrentWindow()->Size.x / zoomLevel), 1), ImGuiTableFlags_ScrollY)) {

		bool createFolder = false;

		if (ImGui::BeginPopupContextWindow(
				"AssetBrowserPopup", ImGuiPopupFlags_NoOpenOverExistingPopup)) {
			if (ImGui::MenuItem("Create Folder")) {
				createFolder = true;
			}
			if (ImGui::MenuItem("Open in File Explorer")) {
				CitronIO::IO::openFileExplorer(currentDirectory.c_str());
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
				CitronIO::IO::createDirectory(currentDirectory / folderName);
				CITRON_CLIENT_INFO("Created new directory {}",
								   (currentDirectory / folderName).string());
				pendingRefreshDirectory = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::Dummy(ImVec2(0.0f, 4.0f));
		int i = 0;

		for (auto &entry : directoryListings) {
			ImGui::TableNextColumn();
			ImGui::PushID(i++);

			if (entry.isDirectory) {
				if (ImGui::Selectable("##Folder", &entry.selected,
									  ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap,
									  ImVec2(zoomLevel * 0.9f, zoomLevel))) {
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						currentDirectory = entry.path;
						pendingRefreshDirectory = true;
					} else {
						entry.selected = !entry.selected;
					}
				}
				ImVec2 rect_min = ImGui::GetItemRectMin();
				ImVec2 rect_max = ImGui::GetItemRectMax();
				ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)view, rect_min, rect_max);

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload *payload =
							ImGui::AcceptDragDropPayload("ASSET_FILE_TRANSFER")) {
						std::string srcPath((const char *)payload->Data,
											payload->DataSize);
						if (srcPath != context.currentlyEditedSceneAssetPath) {
							CitronIO::IO::moveFileOrFolder(srcPath, entry.path);
							appContext.assetManager.moveAsset(srcPath, entry.path);
							pendingRefreshDirectory = true;
							ImGui::PopID();
							continue;
						} else {
							CITRON_CLIENT_ERROR(
								"Cannot move currently opened Scene file");
						}
					}
					ImGui::EndDragDropTarget();
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
						std::filesystem::path newPath = entry.path.parent_path() / folderName;
						CitronIO::IO::renameDirectory(entry.path, newPath);
						ImGui::CloseCurrentPopup();
						pendingRefreshDirectory = true;

						CITRON_CORE_INFO("Renamed folder {} to {}", entry.path.string(),
										 newPath.string());
					}
					if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			} else {
				ImGui::SetWindowFontScale(6.0f * zoomLevel / 150.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
									ImVec2(0.5f, 0.5f));
				if (ImGui::Selectable(ICON_FA_FILE, &entry.selected,
									  ImGuiSelectableFlags_AllowDoubleClick,
									  ImVec2(zoomLevel * 0.9f, zoomLevel))) {
					if (appContext.assetManager.isKnownAssetFileExtension(entry.path.extension().string()))
						assetPropertiesPanel.setSelectedAsset(entry.path);
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						std::string command =
							"start notepad \"" + entry.path.string() + "\"";
						system(command.c_str());
					} else {
						entry.selected = !entry.selected;
					}
				}
				ImGui::PopStyleVar();
				ImGui::SetWindowFontScale(1.0f);

				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("ASSET_FILE_TRANSFER",
											  entry.path.string().data(), entry.path.string().size());
					ImGui::Text("File: %s", entry.name.c_str());
					ImGui::EndDragDropSource();
				}
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
						std::filesystem::path newPath =
							entry.path.parent_path() / (fileName + entry.path.extension().string());
						if (context.currentlyEditedSceneAssetPath == entry.path) {
							context.currentlyEditedSceneAssetPath = newPath;
							appContext.sceneManager.getActiveScene()->rename(fileName);
						}

						CitronIO::IO::renameDirectory(entry.path, newPath);
						ImGui::CloseCurrentPopup();

						CITRON_CORE_INFO("Renamed file {} to {}", entry.path.string(),
										 newPath.string());

						pendingRefreshDirectory = true;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}

			ImGui::SetWindowFontScale(1.0f);
			ImGui::Text("%s", entry.name.c_str());
			ImGui::PopID();
		}
		ImGui::EndTable();
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
					   currentDirectory.string());
	directoryListings.clear();
	for (std::filesystem::path &entry :
		 CitronIO::IO::getEntriesInDirectory(currentDirectory)) {
		if (entry.extension() == ".meta")
			continue;
		AssetCard card = {};
		card.path = entry.string();
		card.name = entry.filename().string();
		card.isDirectory = std::filesystem::is_directory(entry);
		directoryListings.push_back(card);
	}

	appContext.assetManager.refreshAssetRegistry();
}

void AssetPropertiesPanel::onAttach() {}
void AssetPropertiesPanel::onDetach() {}
void AssetPropertiesPanel::onUpdate() {}
void AssetPropertiesPanel::onDraw() {
	ImGui::Begin("Asset Properties");
	if (currentlySelectedAsset != UUID::nullID && currentlySelectedAssetType != CitronAssets::AssetType::UNKNOWN) {
		AssetMetadata metadata = appContext.assetManager.getAssetMetadata(currentlySelectedAssetPath);
		std::shared_ptr<CitronAssets::AssetBase> asset = appContext.assetManager.getAsset<CitronAssets::AssetBase>(currentlySelectedAsset);
		drawGenericProperties(metadata);
		switch (currentlySelectedAssetType) {
		case CitronAssets::AssetType::SHADER:
			drawShaderProperties(std::static_pointer_cast<Shader>(asset));
			break;
		case CitronAssets::AssetType::MATERIAL:
			drawMaterialProperties(std::static_pointer_cast<Material>(asset));
			break;
		case CitronAssets::AssetType::TEXTURE:
			drawTextureProperties(std::static_pointer_cast<Texture>(asset));
			break;
		case CitronAssets::AssetType::MESH:
			drawMeshProperties(std::static_pointer_cast<Mesh>(asset));
			break;
		default:
			break;
		}
	}
	ImGui::End();
}
void AssetPropertiesPanel::onEvent(Event &e) {
}

void AssetPropertiesPanel::setSelectedAsset(const std::filesystem::path &path) {
	AssetMetadata metadata = appContext.assetManager.getAssetMetadata(path);

	currentlySelectedAsset = metadata.uuid;
	currentlySelectedAssetType = metadata.assetType;
	currentlySelectedAssetPath = path;
}

void AssetPropertiesPanel::drawShaderProperties(std::shared_ptr<Shader> shader) {
}

void AssetPropertiesPanel::drawMaterialProperties(std::shared_ptr<Material> material) {
	if (!material)
		return;
	static bool open = true;
	if (InspectorPanel::collapsingHeader("Material", &open)) {
		InspectorPanel::drawAssetReferenceComponentGui<Shader>("Shader", material->shader, appContext);
	}
}

void AssetPropertiesPanel::drawTextureProperties(std::shared_ptr<Texture> texture) {
}

void AssetPropertiesPanel::drawMeshProperties(std::shared_ptr<Mesh> mesh) {
}

void AssetPropertiesPanel::drawGenericProperties(AssetMetadata metadata) {
	if (!appContext.assetManager.isValidAsset(metadata.uuid))
		return;
	static bool open = true;
	if (InspectorPanel::collapsingHeader("Generic", &open)) {
		ImGui::Text("Asset Type: %s", std::string(to_string(metadata.assetType)).c_str());
		ImGui::Text("Asset Path: %s", metadata.assetPath.string().c_str());
		ImGui::Text("Asset UUID: %u", (unsigned int)metadata.uuid);
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
	std::shared_ptr<Scene> currentEditedScene =
		appContext.sceneManager.getActiveScene();
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
	EditorContext &context = Editor::get().getEditorContext();
	CitronECS::EntityBaseComponent &entityBase =
		scene->getRegistry().get<CitronECS::EntityBaseComponent>(entity);

	ImGui::PushID(entityBase.uuid);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
	bool node1_open = ImGui::TreeNodeEx(entityBase.name.c_str(),
										ImGuiTreeNodeFlags_FramePadding);
	ImGui::PopStyleVar();

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("ENTITY_TREE_REORDER",
								  (uint64_t *)&entityBase.uuid,
								  sizeof(uint64_t));
		ImGui::Text("Reparenting Entity: %s", entityBase.name.c_str());
		ImGui::EndDragDropSource();
	} else if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload *payload =
				ImGui::AcceptDragDropPayload("ENTITY_TREE_REORDER")) {
			uint64_t *childEntityUUID = (uint64_t *)payload->Data;
			UUID newChildUUID = *childEntityUUID;
			std::shared_ptr<Scene> currentScene = appContext.sceneManager.getActiveScene();
			currentScene->reparentEntity(
				currentScene->getEntity(newChildUUID),
				currentScene->getEntity(entityBase.uuid));
		}
		ImGui::EndDragDropTarget();
	}

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
	ImGui::SameLine();
	std::string entitySearchResult;
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##EntitySearch", "Search by entity name",
							 &entitySearchResult);

	if (ImGui::BeginTable("LogTable", 1,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

		int i = 0;
		if (currentEditedScene) {
			for (std::shared_ptr<System> &system :
				 currentEditedScene->getSystems()) {
				ImGui::PushID(i++);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Selectable(system->getName().c_str(), false,
								  ImGuiSelectableFlags_SpanAllColumns);
				ImGui::PopID();
			}
		}

		if (currentEditedScene) {
			const auto &view =
				currentEditedScene->getRegistry().view<EntityBaseComponent>();
			for (const entt::entity &entity : view) {
				auto &entityBase = view.get<EntityBaseComponent>(entity);
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
					UUID newEntity = currentEditedScene->createEntity();
					currentEditedScene->getRegistry().emplace<MeshComponent>(
						currentEditedScene->getEntity(newEntity));
					currentEditedScene->getRegistry().emplace<TransformComponent>(
						currentEditedScene->getEntity(newEntity));
				}

				ImGui::EndPopup();
			}
		}

		ImGui::PopStyleVar();

		ImGui::EndTable();
	}

	ImGui::End();
}
void OutlinerPanel::onEvent(Event &e) {}

bool InspectorPanel::collapsingHeader(const char *label, bool *p_open,
									  const char *icon_open,
									  const char *icon_closed) {
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

void InspectorPanel::onAttach() {}
void InspectorPanel::onDetach() {}
void InspectorPanel::onUpdate() {}

template <typename T>
	requires std::derived_from<T, AssetBase>
void InspectorPanel::drawAssetReferenceComponentGui(
	const std::string assetName, AssetReference<T> &assetReference, AppContext appContext) {

	if (assetReference.path == "" && assetReference.uuid != UUID::nullID && appContext.assetManager.isValidAsset(assetReference.uuid)) {
		assetReference.path = appContext.assetManager.getAssetMetadata(assetReference.uuid).assetPath.string();
	}

	EditorContext &context = Editor::get().getEditorContext();
	AssetManager &assetManager =
		appContext.assetManager;

	ImGui::PushID(&assetReference);
	if (ImGui::Button("Clear")) {
		assetReference.uuid = UUID::nullID;
		assetReference.path.clear();
	}
	ImGui::SameLine();
	ImGui::InputText(assetName.c_str(), &assetReference.path,
					 ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload *payload =
				ImGui::AcceptDragDropPayload("ASSET_FILE_TRANSFER")) {
			std::string srcPath((const char *)payload->Data, payload->DataSize);
			AssetMetadata metadata = assetManager.getAssetMetadata(std::filesystem::path(srcPath));
			if (assetManager.isValidAsset(metadata.uuid)) {
				if (metadata.assetType == AssetReference<T>::assetType) {
					assetReference.uuid = metadata.uuid;
					assetReference.path = metadata.assetPath.string();
					appContext.assetManager.getAsset<T>(metadata.uuid);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::PopID();
}

void InspectorPanel::onDraw() {
	ImGui::Begin("Inspector");
	EditorContext &context = Editor::get().getEditorContext();
	auto &registry = appContext.sceneManager.getActiveScene()->getRegistry();
	const entt::entity selectedEntity = context.getCurrentSelectedEntity();
	if (selectedEntity != entt::null && registry.valid(selectedEntity)) {
		if (registry.all_of<EntityBaseComponent>(selectedEntity)) {
			EntityBaseComponent &entityBase =
				registry.get<EntityBaseComponent>(selectedEntity);
			static bool selection = true;
			if (collapsingHeader("Entity Base Component", &selection)) {
				float width = ImGui::GetContentRegionAvail().x;

				ImGui::InputText("Name", &entityBase.name);
				ImGui::Text("ID: %u", (unsigned int)entityBase.uuid);
			}
		}
		if (registry.all_of<TransformComponent>(selectedEntity)) {
			TransformComponent &transformComponent =
				registry.get<TransformComponent>(selectedEntity);
			ImGui::DragFloat3("Position", &transformComponent.position[0]);
			glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(transformComponent.rotation));
			if (ImGui::DragFloat3("Rotation", &eulerRotation[0])) {
				transformComponent.rotation = glm::quat(glm::radians(eulerRotation));
			}
			ImGui::DragFloat3("Scale", &transformComponent.scale[0]);
		}
		if (registry.all_of<MeshComponent>(selectedEntity)) {
			MeshComponent &meshComponent =
				registry.get<MeshComponent>(selectedEntity);
			static bool selection = true;
			if (collapsingHeader("Mesh Component", &selection)) {
				drawAssetReferenceComponentGui<Mesh>("Geometry",
													 meshComponent.meshAsset, appContext);
				drawAssetReferenceComponentGui<Material>("Material",
														 meshComponent.materialAsset, appContext);
			}
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

void AssetRegistryPanel::onAttach() {
}

void AssetRegistryPanel::onDetach() {
}

void AssetRegistryPanel::onUpdate() {
}

void AssetRegistryPanel::onDraw() {
	if (ImGui::Begin("Asset Registry", &showWindow)) {
		for (const auto &[id, AssetMetadata] : appContext.assetManager.getAssetMetadataRegistry()) {
			ImGui::Text("Asset Type: %s", std::string(to_string(AssetMetadata.assetType)).c_str());
			ImGui::Text("Asset Path: %s", AssetMetadata.assetPath.string().c_str());
			ImGui::Text("Asset UUID: %u", (unsigned int)id);
		}
	}
	ImGui::End();
}

void AssetRegistryPanel::onEvent(Event &e) {
}
