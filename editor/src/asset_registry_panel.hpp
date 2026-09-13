#pragma once

#include "panel.hpp"

class AssetRegistryPanel : public Panel {
  public:
	AssetRegistryPanel(AppContext appContext) : Panel("Asset Registry", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	bool showWindow = false;
};
