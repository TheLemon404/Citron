#include "asset_registry_panel.hpp"

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
			ImGui::Separator();
		}
	}
	ImGui::End();
}

void AssetRegistryPanel::onEvent(Event &e) {
}
