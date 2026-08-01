#pragma once

#include "app.hpp"
#include "panel.hpp"
#include <layer.hpp>
#include <renderer.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

class GuiLayer : public Layer {
  public:
	GuiLayer(AppContext appContext);
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void drawGui(wgpu::TextureView &sceneView,
				 CitronGraphics::RenderPass &currentRenderPass);
	void onEvent(Event &e) override;

	ViewPanel viewPanel;
	AssetPanel assetPanel;
	AssetPropertiesPanel assetPropertiesPanel;
	AssetRegistryPanel assetRegistryPanel;
	OutlinerPanel outlinerPanel;
	ConsolePanel consolePanel;
	InspectorPanel inspectorPanel;
	void applyTheme();

  private:
	AppContext appContext;
};
