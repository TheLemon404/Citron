#include "asset_properties_panel.hpp"
#include "inspector_panel.hpp"
#include "gui_elements.hpp"
#include "shader.hpp"

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
			drawTextureProperties(std::static_pointer_cast<ImageTexture>(asset));
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
	if (InspectorPanel::collapsingHeader("Material")) {
		if (ImGui::BeginTable("##ComponentMemberTable", 1,
							  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)) {
			ImGui::TableNextColumn();
			GuiElements::drawAssetReferenceComponentGui<Shader>("Shader", material->shader, appContext);
			ImGui::EndTable();
		}
	}
}

void AssetPropertiesPanel::drawTextureProperties(std::shared_ptr<Texture> texture) {
}

void AssetPropertiesPanel::drawMeshProperties(std::shared_ptr<Mesh> mesh) {
}

void AssetPropertiesPanel::drawGenericProperties(AssetMetadata metadata) {
	if (!appContext.assetManager.isValidAsset(metadata.uuid))
		return;

	if (InspectorPanel::collapsingHeader("Generic")) {
		if (ImGui::BeginTable("##ComponentMemberTable", 1,
							  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)) {
			// First column gets a fixed width of 150 units
			ImGui::TableNextColumn();
			ImGui::Text("Asset Type: %s", std::string(to_string(metadata.assetType)).c_str());
			ImGui::TableNextColumn();
			ImGui::Text("Asset Path: %s", metadata.assetPath.string().c_str());
			ImGui::TableNextColumn();
			ImGui::Text("Asset UUID: %u", (unsigned int)metadata.uuid);
			ImGui::EndTable();
		}
	}
}
