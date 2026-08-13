#pragma once

#include "app.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "logger.hpp"
#include "panel.hpp"
#include "texture.hpp"
#include "view_panel.hpp"
#include <layer.hpp>
#include <memory>
#include <renderer.hpp>
#include <webgpu/webgpu.hpp>

constexpr ImVec4 xColor = ImVec4(1, 0.373, 0.373, 1.0f);
constexpr ImVec4 yColor = ImVec4(0.655, 0.949, 0.267, 1.0f);
constexpr ImVec4 zColor = ImVec4(0.337, 0.596, 0.988, 1.0f);
constexpr ImVec4 wColor = ImVec4(0.698, 0.443, 0.922, 1.0f);
constexpr ImVec4 themeColor = ImVec4(0.737, 0.502, 0.306, 1.0f);
constexpr ImVec4 themeSecondaryColor = ImVec4(0.2784314f, 0.44705883f, 0.7019608f, 1.0f);

using namespace CitronCore;

struct Icon {
	ImRect uv;
	ImVec2 dimensions;
};

class EditorIcons {
  public:
	void initAtlas(const std::filesystem::path atlasPath, Device &device) {
		atlas = ImageTexture::loadFromFile(atlasPath, device);
		if (!atlas) {
			CITRON_CLIENT_CRITICAL("Failed to load editor icon atlas texture");
			throw std::runtime_error("Failed to load atlas texture");
		}
	}

	void registerIcon(std::string name, ImRect uv, ImVec2 dimensions) {
		icons[name] = {uv, dimensions};
	}
	Icon getIcon(std::string name) const {
		return icons.at(name);
	}

	ImVec2i getAtlasSize() const { return ImVec2i(atlas->getWidth(), atlas->getHeight()); }

	const wgpu::TextureView &getTextureView() const { return atlas->getTextureView(); }

  private:
	std::unordered_map<std::string, Icon> icons;
	std::shared_ptr<ImageTexture> atlas;
};

class GuiLayer : public Layer {
  public:
	GuiLayer(AppContext appContext);
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender(void *frame, void *renderableData) override;
	void drawGui(Texture &editorView, Texture &gameView,
				 CitronGraphics::RenderPass &currentRenderPass);
	void onEvent(Event &e) override;

	Texture editorViewTextureRenderTarget;

	EditorIcons icons;

	ViewPanel viewPanel;
	AssetPanel assetPanel;
	AssetPropertiesPanel assetPropertiesPanel;
	AssetRegistryPanel assetRegistryPanel;
	OutlinerPanel outlinerPanel;
	ConsolePanel consolePanel;
	InspectorPanel inspectorPanel;
	GamePanel gamePanel;
	void applyTheme();

  private:
	AppContext appContext;
};
