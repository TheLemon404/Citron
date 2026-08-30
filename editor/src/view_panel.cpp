#include "ImOGuizmo.hpp"
#include "app.hpp"
#include "clock.hpp"
#include "debug.hpp"
#include "imgui_internal.h"
#include "mesh.hpp"
#include "panel.hpp"
#include "uuid.hpp"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>
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
	for (const auto &entity : appContext.sceneManager.getActiveScene()->getRegistry().view<TransformComponent, PerspectiveCameraComponent>()) {
		PerspectiveCameraComponent &camera = appContext.sceneManager.getActiveScene()->getRegistry().get<PerspectiveCameraComponent>(entity);
		glm::vec3 globalPosition = appContext.sceneManager.getActiveScene()->getGlobalPosition(entity);
		glm::vec3 cameraRight = glm::cross(camera.view.up, camera.view.forward) * camera.view.aspect;
		glm::vec3 forwardPoint = globalPosition + camera.view.forward;
		glm::vec3 frustrumCorners[4] = {
			forwardPoint + cameraRight + camera.view.up,
			forwardPoint - cameraRight + camera.view.up,
			forwardPoint - cameraRight - camera.view.up,
			forwardPoint + cameraRight - camera.view.up,
		};

		DebugUtils::addDebugLine(globalPosition, frustrumCorners[0]);
		DebugUtils::addDebugLine(globalPosition, frustrumCorners[1]);
		DebugUtils::addDebugLine(globalPosition, frustrumCorners[2]);
		DebugUtils::addDebugLine(globalPosition, frustrumCorners[3]);
		DebugUtils::addDebugLine(frustrumCorners[0], frustrumCorners[1]);
		DebugUtils::addDebugLine(frustrumCorners[1], frustrumCorners[2]);
		DebugUtils::addDebugLine(frustrumCorners[2], frustrumCorners[3]);
		DebugUtils::addDebugLine(frustrumCorners[3], frustrumCorners[0]);
	}

	if (currentlySelectedItem.index() == 0 && std::get<entt::entity>(currentlySelectedItem) != entt::null) {
		entt::entity entity = std::get<entt::entity>(currentlySelectedItem);
		if (appContext.sceneManager.getActiveScene()->getRegistry().any_of<MeshComponent>(entity)) {
			std::shared_ptr<Mesh> entityMesh = appContext.assetManager.getAsset<Mesh>(appContext.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity).meshAsset.uuid);
			glm::vec3 minBounds = entityMesh->getBoundsMin();
			glm::vec3 maxBounds = entityMesh->getBoundsMax();
			glm::vec3 globalPosition = appContext.sceneManager.getActiveScene()->getGlobalPosition(entity);
			DebugUtils::addDebugCube(globalPosition - (maxBounds - minBounds) / 2.0f, globalPosition + (maxBounds - minBounds) / 2.0f);
		}
	}

	if (!focused || !viewportMovementActive)
		return;

	PerspectiveView &editorView = Editor::get().editorView;
	float deltaTime = CitronCore::Clock::getDeltaTime();

	CitronInput::InputLayer *inputLayer = Editor::get().getLayer<CitronInput::InputLayer>();
	if (inputLayer->isPressed(SDLK_W)) {
		editorView.position += editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}
	if (inputLayer->isPressed(SDLK_S)) {
		editorView.position -= editorView.forward * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}
	if (inputLayer->isPressed(SDLK_D)) {
		editorView.position += glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}
	if (inputLayer->isPressed(SDLK_A)) {
		editorView.position -= glm::normalize(glm::cross(editorView.forward, glm::vec3(0.0f, 1.0f, 0.0f))) * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}
	if (inputLayer->isPressed(SDLK_E)) {
		editorView.position += globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}
	if (inputLayer->isPressed(SDLK_Q)) {
		editorView.position -= globalUp * (inputLayer->isPressed(SDLK_LSHIFT) ? motionSettings.fastMoveSpeed : motionSettings.moveSpeed) * deltaTime;
	}

	if (pendingBuildScripts) {
		Editor::get().getContext().scriptingEngine.buildScripts(Editor::get().getEditorContext().projectFilePath.parent_path());
		CITRON_CLIENT_INFO("Building Scripts");
		pendingBuildScripts = false;
	}
}

void ViewPanel::onDraw() {
	PerspectiveView &editorView = Editor::get().editorView;
	EditorContext &editorContext = Editor::get().getEditorContext();

	EditorIcons &icons = Editor::get().getLayer<GuiLayer>()->editorIcons;

	ImGui::Begin("Viewport", nullptr);
	viewportSize = ImGui::GetContentRegionAvail();
	ImVec2 viewportPos = ImGui::GetCursorScreenPos();
	focused = ImGui::IsWindowFocused();
	if (viewportMovementActive)
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

	WGPUTextureView view = sceneView;
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((ImTextureID)(uintptr_t)view, viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y));
	if (editorContext.getPlaymodeState() != EditorPlaymodeState::Stopped)
		drawList->AddRect(viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y), ImColor(themeSecondaryColor), 0, 0, 2.0f);
	float imoguizmoSize = 120.0f;
	ImOGuizmo::config.axisLengthScale = 0.1f;
	ImOGuizmo::SetRect(viewportPos.x + viewportSize.x - imoguizmoSize, viewportPos.y, imoguizmoSize);

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Header, themeSecondaryColor);

	float toolbarWidth = 40.0f;

	if (ImGui::BeginChild("##Actions", ImVec2(toolbarWidth, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar)) {
		WGPUTextureView iconView = icons.getTextureView();

		if (ImGui::Selectable("##Build", false, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			// build scripts
			pendingBuildScripts = true;
		}
		ImVec2 rectMin = ImGui::GetItemRectMin();
		ImVec2 rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Build").uv.Min, icons.getIcon("Build").uv.Max);
		if (ImGui::Selectable("##Play", editorContext.getPlaymodeState() == EditorPlaymodeState::Playing, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			if (editorContext.getPlaymodeState() == EditorPlaymodeState::Playing) {
				Editor::get().stopPlaying();
			} else if (editorContext.getPlaymodeState() == EditorPlaymodeState::Paused) {
				Editor::get().stopPlaying();
				Editor::get().startPlaying();
			} else {
				Editor::get().startPlaying();
			}
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Play").uv.Min, icons.getIcon("Play").uv.Max);
		if (ImGui::Selectable("##Pause", editorContext.getPlaymodeState() == EditorPlaymodeState::Paused, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			if (editorContext.getPlaymodeState() == EditorPlaymodeState::Playing) {
				Editor::get().pausePlaying();
			} else if (editorContext.getPlaymodeState() == EditorPlaymodeState::Paused) {
				Editor::get().resumePlaying();
			}
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Pause").uv.Min, icons.getIcon("Pause").uv.Max);
		if (ImGui::Selectable("##Translate", manipulationSettings.currentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			manipulationSettings.currentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Translate").uv.Min, icons.getIcon("Translate").uv.Max);
		if (ImGui::Selectable("##Rotate", manipulationSettings.currentGizmoOperation == ImGuizmo::OPERATION::ROTATE, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			manipulationSettings.currentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Rotate").uv.Min, icons.getIcon("Rotate").uv.Max);
		if (ImGui::Selectable("##Scale", manipulationSettings.currentGizmoOperation == ImGuizmo::OPERATION::SCALE, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			manipulationSettings.currentGizmoOperation = ImGuizmo::OPERATION::SCALE;
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Scale").uv.Min, icons.getIcon("Scale").uv.Max);
		if (ImGui::Selectable("##Snap", manipulationSettings.snap, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			manipulationSettings.snap = !manipulationSettings.snap;
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Snap").uv.Min, icons.getIcon("Snap").uv.Max);
		if (ImGui::Selectable("##Local", manipulationSettings.relativeSpaceMode == ImGuizmo::MODE::LOCAL, ImGuiSelectableFlags_None, ImVec2(toolbarWidth, toolbarWidth))) {
			if (manipulationSettings.relativeSpaceMode == ImGuizmo::MODE::WORLD)
				manipulationSettings.relativeSpaceMode = ImGuizmo::MODE::LOCAL;
			else
				manipulationSettings.relativeSpaceMode = ImGuizmo::MODE::WORLD;
		}
		rectMin = ImGui::GetItemRectMin();
		rectMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)(uintptr_t)iconView, rectMin, rectMax, icons.getIcon("Local").uv.Min, icons.getIcon("Local").uv.Max);
	}
	ImGui::EndChild();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	glm::mat4 viewMatrix = editorView.getViewMatrix();
	glm::mat4 projMat = editorView.getProjectionMatrix();

	entt::registry &registry = appContext.sceneManager.getActiveScene()->getRegistry();
	if (currentlySelectedItem.index() == 0 && registry.valid(std::get<entt::entity>(currentlySelectedItem)) && registry.any_of<TransformComponent, EntityBaseComponent>(std::get<entt::entity>(currentlySelectedItem))) {
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::Enable(true);
		TransformComponent &transform = appContext.sceneManager.getActiveScene()->getRegistry().get<TransformComponent>(std::get<entt::entity>(currentlySelectedItem));
		editTransformComponent(viewportPos, viewportSize, &viewMatrix[0][0], &projMat[0][0], std::get<entt::entity>(currentlySelectedItem));
	}
	editorView.aspect = viewportSize.x / viewportSize.y;

	ImOGuizmo::DrawGizmo(&viewMatrix[0][0], &projMat[0][0]);
	ImGui::End();
}

void ViewPanel::onEvent(Event &e) {
	if (!focused)
		return;

	PerspectiveView &editorView = Editor::get().editorView;

	float deltaTime = CitronCore::Clock::getDeltaTime();

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
