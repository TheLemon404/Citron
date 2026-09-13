#pragma once

#include "panel.hpp"

class AssetPropertiesPanel : public Panel {
  public:
	AssetPropertiesPanel(AppContext appContext) : Panel("Asset Properties", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	void setSelectedAsset(const std::filesystem::path &path);

  private:
	void drawShaderProperties(std::shared_ptr<Shader> shader);
	void drawMaterialProperties(std::shared_ptr<Material> material);
	void drawTextureProperties(std::shared_ptr<Texture> texture);
	void drawMeshProperties(std::shared_ptr<Mesh> mesh);
	void drawGenericProperties(AssetMetadata metadata);

	UUID currentlySelectedAsset = UUID::nullID;
	AssetType currentlySelectedAssetType = CitronAssets::AssetType::UNKNOWN;
	std::filesystem::path currentlySelectedAssetPath = "";
};
