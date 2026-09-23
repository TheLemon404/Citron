#include "asset_panel.hpp"
#include "asset_defaults.hpp"
#include "editor.hpp"
#include "gui.hpp"
#include "imgui.h"
#include "keyboard.hpp"

#include <io.hpp>
#include <string>
#include <imgui_stdlib.h>

void AssetPanel::onAttach() {
	EditorContext &context = Editor::get().getEditorContext();
	currentDirectory = context.projectFilePath.parent_path() / "Assets";
	refreshDirectoryListings();
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
			currentDirectory != context.projectFilePath.parent_path() / "Assets") {
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

	EditorIcons &fileIcons = Editor::get().getLayer<GuiLayer>()->fileIcons;
	WGPUTextureView iconView = fileIcons.getTextureView();
	if (ImGui::BeginTable("##AssetBrowserTable", std::max((int)(ImGui::GetCurrentWindow()->Size.x / zoomLevel), 1), ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit)) {

		bool createFolder = false;

		if (ImGui::BeginPopupContextWindow(
				"AssetBrowserPopup", ImGuiPopupFlags_NoOpenOverExistingPopup)) {
			if (ImGui::MenuItem("Create Folder")) {
				createFolder = true;
			}
			if (ImGui::MenuItem("Open in File Explorer")) {
				CitronIO::IO::openFileExplorer(currentDirectory.c_str());
			}
			if (ImGui::BeginMenu("Create")) {
				if (ImGui::MenuItem("Shader")) {
					CitronIO::IO::createFile(currentDirectory / "test.wgsl");
					CitronIO::IO::writeFile(currentDirectory / "test.wgsl", AssetDefaults::getDefaultShader());
					refreshDirectoryListings();
				}
				if (ImGui::MenuItem("Material")) {
					CitronIO::IO::createFile(currentDirectory / "test.mat");
					refreshDirectoryListings();
				}
				ImGui::EndMenu();
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
				ImVec2 rectMin = ImGui::GetItemRectMin();
				ImVec2 rectMax = ImGui::GetItemRectMax();
				Icon folderIcon = fileIcons.getIcon("Folder");
				ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, folderIcon.uv.Min, folderIcon.uv.Max);

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
				if (ImGui::Selectable("##File", &entry.selected,
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
				ImVec2 rect_min = ImGui::GetItemRectMin();
				ImVec2 rect_max = ImGui::GetItemRectMax();
				std::string iconType = "GenericFile";
				const std::string fileExtension = entry.path.extension().string();
				if (fileExtension == ".cpp" || fileExtension == ".hpp") {
					iconType = "C++";
				} else if (fileExtension == ".cs") {
					iconType = "C#";
				} else if (fileExtension == ".mat") {
					iconType = "Material";
				} else if (fileExtension == ".wgsl") {
					iconType = "Shader";
				}
				Icon fileIcon = fileIcons.getIcon(iconType);
				ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rect_min, rect_max, fileIcon.uv.Min, fileIcon.uv.Max);

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
