#pragma once

#include "component.hpp"
#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "panel.hpp"

struct ViewportMotionSettings {
	float moveSpeed = 0.5f;
	float fastMoveSpeed = 1.0f;
	float lookSpeed = 0.005f;
};

class ViewPanel : public Panel {
  public:
	ViewPanel(AppContext appContext, entt::entity &currentlySelectedEntity, PerspectiveView &editorView) : Panel("Viewport", appContext), currentlySelectedEntity(currentlySelectedEntity), editorView(editorView) {};

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	void setView(wgpu::TextureView sceneView) {
		this->sceneView = sceneView;
	}

  private:
	void editTransformComponent(ImVec2 viewportPos, ImVec2 viewRectSize, float *cameraView, float *cameraProjection, TransformComponent &transform);
	bool mouseSelectEvent(Event &e);

	entt::entity &currentlySelectedEntity;

	bool focused = false;
	ViewportMotionSettings motionSettings;
	PerspectiveView &editorView;
	bool viewportMovementActive = false;
	wgpu::TextureView sceneView;
};
