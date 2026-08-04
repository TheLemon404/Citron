#pragma once

#include <assets.hpp>
#include <app.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "editor.hpp"

class GuiElements {
  public:
	template <typename T>
	static void drawAssetReferenceComponentGui(const std::string &assetName, AssetReference<T> &assetReference, AppContext appContext) {
		if (assetReference.path == "" && assetReference.uuid != UUID::nullID && appContext.assetManager.isValidAsset(assetReference.uuid)) {
			assetReference.path = appContext.assetManager.getAssetMetadata(assetReference.uuid).assetPath.string();
		}

		EditorContext &context = Editor::get().getEditorContext();
		AssetManager &assetManager =
			appContext.assetManager;

		ImGui::PushID(&assetReference);

		std::string assetNameStr = std::filesystem::path(assetReference.path).filename().string();
		ImGui::InputText(assetName.c_str(), &assetNameStr,
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
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			assetReference.uuid = UUID::nullID;
			assetReference.path.clear();
		}
		ImGui::PopID();
	}
};
