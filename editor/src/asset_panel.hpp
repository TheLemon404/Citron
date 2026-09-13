#pragma once

#include "panel.hpp"
#include "asset_properties_panel.hpp"

class AssetPanel : public Panel {
  public:
	AssetPanel(AppContext appContext, AssetPropertiesPanel &assetPropertiesPanel) : Panel("Assets", appContext), assetPropertiesPanel(assetPropertiesPanel) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	std::filesystem::path currentDirectory = "";
	bool pendingRefreshDirectory = false;

  private:
	AssetPropertiesPanel &assetPropertiesPanel;

	void refreshDirectoryListings();
	std::vector<AssetCard> directoryListings;

	int zoomLevel = 100;
};
