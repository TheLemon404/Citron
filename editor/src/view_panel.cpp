#include "uuid.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "SDL3/SDL_mouse.h"
#include "component.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "event.hpp"
#include "glm/ext/vector_float3.hpp"
#include "gui.hpp"
#include "imgui.h"
#include "input.hpp"
#include "mouse.hpp"
#include "view.hpp"
#include "view_panel.hpp"
#include <ImGuizmo.h>
#include <iso646.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>

constexpr glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

using namespace CitronCore;
using namespace CitronECS;

void ViewPanel::onAttach() {
}

void ViewPanel::onDetach() {
}

void ViewPanel::onUpdate() {
	if (!focused || !viewportMovementActive)
		return;

	CitronInput::InputLayer *inputLayer = Editor::get().getLayer<CitronInput::InputLayer>();
	if (inputLayer->isPressed(SDLK_W)) {
		editorView.position += editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_S)) {
		editorView.position -= editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_D)) {
		editorView.position += glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_A)) {
		editorView.position -= glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_E)) {
		editorView.position += globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
	if (inputLayer->isPressed(SDLK_Q)) {
		editorView.position -= globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed);
	}
}

void ViewPanel::onDraw() {
	ImGui::Begin("Viewport", nullptr);
	viewportSize = ImGui::GetContentRegionAvail();
	ImVec2 viewportPos = ImGui::GetCursorScreenPos();
	focused = ImGui::IsWindowFocused();
	if (viewportMovementActive)
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

	WGPUTextureView view = sceneView;
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((ImTextureID)(uintptr_t)view, viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y));

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Header, themeColor);

	ImGui::Dummy(ImVec2(10, 10));
	ImGui::Dummy(ImVec2(10, 10));
	ImGui::SameLine();
	if (ImGui::Selectable("World", manipulationSettings.relativeSpaceMode == ImGuizmo::MODE::WORLD, ImGuiSelectableFlags_None, ImVec2(40.0f, 18.0f))) {
		manipulationSettings.relativeSpaceMode = ImGuizmo::MODE::WORLD;
	}
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5, 0));
	ImGui::SameLine();
	if (ImGui::Selectable("Local", manipulationSettings.relativeSpaceMode == ImGuizmo::MODE::LOCAL, ImGuiSelectableFlags_None, ImVec2(40.0f, 18.0f))) {
		manipulationSettings.relativeSpaceMode = ImGuizmo::MODE::LOCAL;
	}
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5, 0));
	ImGui::SameLine();
	if (ImGui::Selectable("Snap", manipulationSettings.snap, ImGuiSelectableFlags_None, ImVec2(40.0f, 18.0f))) {
		manipulationSettings.snap = !manipulationSettings.snap;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (currentlySelectedEntity != entt::null && appContext.sceneManager.getActiveScene()->getRegistry().any_of<TransformComponent>(currentlySelectedEntity)) {
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::Enable(true);
		TransformComponent &transform = appContext.sceneManager.getActiveScene()->getRegistry().get<TransformComponent>(currentlySelectedEntity);
		editTransformComponent(viewportPos, viewportSize, &editorView.getViewMatrix()[0][0], &editorView.getProjectionMatrix()[0][0], currentlySelectedEntity);
	}
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
			editorView.forward = glm::rotate(editorView.forward, -dx, globalUp);
			glm::vec3 localRightVector = glm::normalize(glm::cross(editorView.forward, globalUp));
			editorView.forward = glm::rotate(editorView.forward, -dy, localRightVector);
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

void ViewPanel::editTransformComponent(ImVec2 viewportPos, ImVec2 viewRectSize, float *cameraView, float *cameraProjection, entt::entity entity) {
	if (ImGui::IsKeyPressed(ImGuiKey_T) && !viewportMovementActive && focused)
		manipulationSettings.currentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E) && !viewportMovementActive && focused)
		manipulationSettings.currentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R) && !viewportMovementActive && focused)
		manipulationSettings.currentGizmoOperation = ImGuizmo::SCALE;

	if (ImGui::IsKeyPressed(ImGuiKey_S) && !viewportMovementActive && focused)
		manipulationSettings.snap = !manipulationSettings.snap;
	glm::vec3 snap;
	switch (manipulationSettings.currentGizmoOperation) {
	case ImGuizmo::TRANSLATE:
		snap = glm::vec3(motionSettings.snapTranslation);
		break;
	case ImGuizmo::ROTATE:
		snap = glm::vec3(motionSettings.snapRotation);
		break;
	case ImGuizmo::SCALE:
		snap = glm::vec3(motionSettings.snapScale);
		break;
	default:
		break;
	}

	std::shared_ptr<Scene> activeScene = appContext.sceneManager.getActiveScene();
	TransformComponent &transform = activeScene->getRegistry().get<TransformComponent>(entity);
	EntityBaseComponent &base = activeScene->getRegistry().get<EntityBaseComponent>(entity);

	ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewRectSize.x, viewRectSize.y);
	glm::mat4 globalParentMatrix = base.parentId != UUID::nullID ? activeScene->getGlobalTransform(activeScene->getEntity(base.parentId)) : glm::mat4(1.0f);
	glm::mat4 matrix = globalParentMatrix * glm::translate(glm::mat4(1.0f), transform.position) * glm::mat4_cast(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.scale);
	glm::mat4 deltaMatrix(1.0f);

	if (ImGuizmo::Manipulate(cameraView, cameraProjection, manipulationSettings.currentGizmoOperation, manipulationSettings.relativeSpaceMode, &matrix[0][0], &deltaMatrix[0][0], manipulationSettings.snap ? &snap.x : nullptr)) {
		if (manipulationSettings.currentGizmoOperation == ImGuizmo::ROTATE) {
			glm::quat deltaRotation = glm::quat_cast(glm::mat3(deltaMatrix));
			glm::quat parentRotation = glm::quat_cast(glm::mat3(globalParentMatrix));
			glm::quat localDeltaRotation = glm::inverse(parentRotation) * deltaRotation * parentRotation;
			transform.rotation = glm::normalize(localDeltaRotation * transform.rotation);
		} else {
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::quat orientation;
			glm::mat4 localMatrix = glm::inverse(globalParentMatrix) * matrix;
			glm::decompose(localMatrix, transform.scale, orientation, transform.position, skew, perspective);
		}
	}
}

bool ViewPanel::mouseSelectEvent(Event &e) {
	MouseButtonPressedEvent &event = static_cast<MouseButtonPressedEvent &>(e);
	if (event.getButton() == SDL_BUTTON_LEFT) {
		CITRON_CLIENT_INFO("Need to implement mouse picking...");
	}

	return false;
}
