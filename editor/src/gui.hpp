#pragma once

#include "app.hpp"
#include "panel.hpp"
#include "view_panel.hpp"
#include <layer.hpp>
#include <renderer.hpp>
#include <webgpu/webgpu.hpp>

constexpr ImVec4 xColor = ImVec4(1, 0.373, 0.373, 1.0f);
constexpr ImVec4 yColor = ImVec4(0.655, 0.949, 0.267, 1.0f);
constexpr ImVec4 zColor = ImVec4(0.337, 0.596, 0.988, 1.0f);
constexpr ImVec4 themeColor = ImVec4(0.2784314f, 0.44705883f, 0.7019608f, 1.0f);
constexpr ImVec4 themeSecondaryColor = ImVec4(0.737, 0.502, 0.306, 1.0f);

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
