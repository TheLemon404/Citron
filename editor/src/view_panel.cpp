#include "view_panel.hpp"
#include "SDL3/SDL_mouse.h"
#include "component.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "event.hpp"
#include "imgui.h"
#include "input.hpp"
#include "mouse.hpp"
#include "view.hpp"
#include <ImGuizmo.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

constexpr glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

using namespace CitronCore;
using namespace CitronECS;

void ViewPanel::onAttach() {
}

void ViewPanel::onDetach() {
}

void ViewPanel::onUpdate() {
	if (!focused)
		return;

	CitronInput::InputLayer *inputLayer = Editor::get().getLayer<CitronInput::InputLayer>();
	if (inputLayer->isPressed(SDLK_W)) {
		editorView.position += editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_S)) {
		editorView.position -= editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_A)) {
		editorView.position += glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_D)) {
		editorView.position -= glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_Q)) {
		editorView.position += globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_E)) {
		editorView.position -= globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
}

void ViewPanel::onDraw() {
	ImGui::Begin("Viewport", nullptr);
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	ImVec2 viewportPos = ImGui::GetCursorScreenPos();

	focused = ImGui::IsWindowFocused();
	if (viewportMovementActive)
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

	if (currentlySelectedEntity != entt::null) {
		TransformComponent &transform = appContext.sceneManager.getActiveScene()->getRegistry().get<TransformComponent>(currentlySelectedEntity);
		View &activeEditorView = Editor::get().getActiveView();
		editTransformComponent(viewportPos, viewportSize, &activeEditorView.getViewMatrix()[0][0], &activeEditorView.getProjectionMatrix()[0][0], transform);
	}

	WGPUTextureView view = sceneView;
	ImGui::Image((ImTextureID)(uintptr_t)view, viewportSize);
	editorView.aspect = viewportSize.x / viewportSize.y;
	ImGui::End();
}

void ViewPanel::onEvent(Event &e) {
	if (!focused)
		return;

	EventDispatcher dispatcher(e);
	dispatcher.dispatch<MouseButtonPressedEvent>(CITRON_BIND_EVENT_FN(ViewPanel::mouseSelectEvent));

	if (e.isInCategory(EventCategoryMouse)) {
		if (e.getEventType() == EventType::MouseMoved && viewportMovementActive) {
			MouseMovedEvent &mouseEvent = static_cast<MouseMovedEvent &>(e);
			float dx = mouseEvent.getDx() * motionSettings.lookSpeed;
			float dy = mouseEvent.getDy() * motionSettings.lookSpeed;
			editorView.forward = glm::rotate(editorView.forward, dx, globalUp);
			glm::vec3 localRightVector = glm::normalize(glm::cross(editorView.forward, globalUp));
			editorView.forward = glm::rotate(editorView.forward, dy, localRightVector);
		}
		if (e.getEventType() == EventType::MouseButtonPressed) {
			MouseButtonPressedEvent &mouseEvent = static_cast<MouseButtonPressedEvent &>(e);
			if (mouseEvent.getButton() == SDL_BUTTON_RIGHT) {
				viewportMovementActive = true;
			}
		}
		if (e.getEventType() == EventType::MouseButtonReleased) {
			MouseButtonReleasedEvent &mouseEvent = static_cast<MouseButtonReleasedEvent &>(e);
			if (mouseEvent.getButton() == SDL_BUTTON_RIGHT) {
				viewportMovementActive = false;
			}
		}
	}
}

void ViewPanel::editTransformComponent(ImVec2 viewportPos, ImVec2 viewRectSize, float *cameraView, float *cameraProjection, TransformComponent &transform) {
	ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewRectSize.x, viewRectSize.y);
	ImGuizmo::Manipulate(cameraView, cameraProjection, ImGuizmo::ROTATE, ImGuizmo::WORLD, &transform.matrix[0][0]);
}

bool ViewPanel::mouseSelectEvent(Event &e) {
	MouseButtonPressedEvent &event = static_cast<MouseButtonPressedEvent &>(e);
	if (event.getButton() == SDL_BUTTON_LEFT) {
		CITRON_CLIENT_INFO("Need to implement mouse picking...");
	}

	return true;
}
