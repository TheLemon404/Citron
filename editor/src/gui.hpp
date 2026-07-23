#pragma once

#include "panel.hpp"
#include <layer.hpp>
#include <renderer.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;

class GuiLayer : public Layer {
  public:
	GuiLayer() : Layer("GuiLayer") {};
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void drawGui(wgpu::TextureView &sceneView,
				 CitronGraphics::RenderPass &currentRenderPass);
	void onEvent(Event &e) override;

	AssetPanel assetPanel;
	OutlinerPanel outlinerPanel;
	ConsolePanel consolePanel;
	InspectorPanel inspectorPanel;
	void applyTheme();
};
