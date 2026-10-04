#pragma once

#include "panel.hpp"

class InspectorPanel : public Panel {
  public:
	InspectorPanel(AppContext appContext) : Panel("Inspector", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	template <typename T>
		requires std::derived_from<T, AssetBase>
	static void drawAssetReferenceComponentGui(const std::string assetName,
											   AssetReference<T> &assetReference,
											   AppContext appContext);

	static bool collapsingHeader(const char *label,
								 const char *icon_open = "",
								 const char *icon_closed = "");

  private:
	void drawComponentIcon(const std::string &name, WGPUTextureView iconView, ImVec2 iconMin, ImVec2 iconMax);
};
