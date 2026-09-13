#pragma once

#include "panel.hpp"

class GamePanel : public Panel {
  public:
	GamePanel(AppContext appContext) : Panel("Game", appContext) {
		viewportSize.x = appContext.window.getWidth();
		viewportSize.y = appContext.window.getHeight();
	}

	void setView(wgpu::TextureView sceneView) {
		this->sceneView = sceneView;
	}

	glm::ivec2 getViewportSize() {
		return {viewportSize.x, viewportSize.y};
	}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

  private:
	bool shouldBeFocused = false;
	bool onPlaymodeEvent(Event &e);

	ImVec2 viewportSize;
	wgpu::TextureView sceneView;
};
