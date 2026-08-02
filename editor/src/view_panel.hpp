#pragma once

#include "component.hpp"
#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "panel.hpp"
#include <ImGuizmo.h>

struct ViewportMotionSettings {
	float moveSpeed = 0.5f;
	float fastMoveSpeed = 1.0f;
	float lookSpeed = 0.005f;
	float snapTranslation = 1.0f;
	float snapRotation = 15.0f;
	float snapScale = 0.25f;
};

struct ViewportManipulationSettings {
	ImGuizmo::MODE relativeSpaceMode = ImGuizmo::MODE::WORLD;
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	bool snap = false;
};

class ViewPanel : public Panel {
  public:
	ViewPanel(AppContext appContext, entt::entity &currentlySelectedEntity, PerspectiveView &editorView) : Panel("Viewport", appContext), currentlySelectedEntity(currentlySelectedEntity), editorView(editorView) {
		viewportSize.x = appContext.window.getWidth();
		viewportSize.y = appContext.window.getHeight();
	};

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	void setView(wgpu::TextureView sceneView) {
		this->sceneView = sceneView;
	}

	glm::vec2 getViewportSize() {
		return {viewportSize.x, viewportSize.y};
	}

  private:
	ImVec2 viewportSize;
	void editTransformComponent(ImVec2 viewportPos, ImVec2 viewRectSize, float *cameraView, float *cameraProjection, entt::entity);
	bool mouseSelectEvent(Event &e);

	entt::entity &currentlySelectedEntity;

	bool focused = false;
	ViewportMotionSettings motionSettings;
	ViewportManipulationSettings manipulationSettings;
	PerspectiveView &editorView;
	bool viewportMovementActive = false;
	wgpu::TextureView sceneView;
};
