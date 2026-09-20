#include "ImOGuizmo.hpp"
#include "SDL3/SDL_keycode.h"
#include "app.hpp"
#include "clock.hpp"
#include "debug.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "imgui_internal.h"
#include "keyboard.hpp"
#include "logger.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "panel.hpp"
#include "uuid.hpp"
#include <unordered_set>
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
		glm::vec3 cameraRight = glm::normalize(glm::cross(camera.view.up, camera.view.forward)) * camera.view.aspect * (camera.view.fov / 90.0f);
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

	// draw bounding boxes for primary selection
	if (currentlySelectedItem.index() == 0 && std::get<entt::entity>(currentlySelectedItem) != entt::null) {
		entt::entity entity = std::get<entt::entity>(currentlySelectedItem);
		if (appContext.sceneManager.getActiveScene()->getRegistry().any_of<MeshComponent>(entity) && appContext.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity).meshAsset.uuid != UUID::nullID) {
			std::shared_ptr<Mesh> entityMesh = appContext.assetManager.getAsset<Mesh>(appContext.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity).meshAsset.uuid);
			glm::vec4 minBounds = glm::vec4(entityMesh->getBoundsMin(), 1.0f);
			glm::vec4 maxBounds = glm::vec4(entityMesh->getBoundsMax(), 1.0f);
			glm::mat4 globalTransform = appContext.sceneManager.getActiveScene()->getGlobalTransform(entity);
			DebugUtils::addDebugCube(glm::xyz(globalTransform * minBounds), glm::xyz(globalTransform * maxBounds), {1.0, 0.4, 0.0});
		}
	}
	// draw bounding boxes for group selection
	for (std::variant<entt::entity, std::shared_ptr<System>> secondary : Editor::get().getEditorContext().getSecondarySelectedItems()) {
		if (secondary.index() == 0) {
			entt::entity entity = std::get<entt::entity>(secondary);
			if (appContext.sceneManager.getActiveScene()->getRegistry().any_of<MeshComponent>(entity) && appContext.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity).meshAsset.uuid != UUID::nullID) {
				std::shared_ptr<Mesh> entityMesh = appContext.assetManager.getAsset<Mesh>(appContext.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity).meshAsset.uuid);
				glm::vec4 minBounds = glm::vec4(entityMesh->getBoundsMin(), 1.0f);
				glm::vec4 maxBounds = glm::vec4(entityMesh->getBoundsMax(), 1.0f);
				glm::mat4 globalTransform = appContext.sceneManager.getActiveScene()->getGlobalTransform(entity);
				DebugUtils::addDebugCube(glm::xyz(globalTransform * minBounds), glm::xyz(globalTransform * maxBounds), {0.8, 0.55, 0.0});
			}
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
	viewportPos = ImGui::GetCursorScreenPos();
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

		editMultiTransform(viewportPos, viewportSize, &viewMatrix[0][0], &projMat[0][0], std::get<entt::entity>(currentlySelectedItem), secondarySelectedItems);
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

			editorView.up = glm::normalize(glm::cross(localRightVector, editorView.forward));
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

void ViewPanel::editMultiTransform(ImVec2 viewportPos, ImVec2 viewRectSize, float *cameraView, float *cameraProjection, entt::entity primaryEntity, std::unordered_set<std::variant<entt::entity, std::shared_ptr<System>>> &secondaryItems) {
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
	TransformComponent &primaryTransform = activeScene->getRegistry().get<TransformComponent>(primaryEntity);
	EntityBaseComponent &primaryBase = activeScene->getRegistry().get<EntityBaseComponent>(primaryEntity);
	ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewRectSize.x, viewRectSize.y);
	glm::mat4 globalParentMatrix = primaryBase.parentId != UUID::nullID ? activeScene->getGlobalTransform(activeScene->getEntity(primaryBase.parentId)) : glm::mat4(1.0f);

	glm::vec3 centerPosition = primaryTransform.position;
	size_t objectCount = 1;

	for (auto &secondaryItem : secondaryItems) {
		if (secondaryItem.index() == 0 && activeScene->getRegistry().any_of<TransformComponent>(std::get<entt::entity>(secondaryItem))) {
			TransformComponent &secondaryTransform = activeScene->getRegistry().get<TransformComponent>(std::get<entt::entity>(secondaryItem));
			centerPosition += secondaryTransform.position;
			objectCount++;
		}
	}

	centerPosition /= objectCount;

	glm::mat4 matrix = globalParentMatrix * glm::translate(glm::mat4(1.0f), centerPosition) * glm::mat4_cast(primaryTransform.rotation) * glm::scale(glm::mat4(1.0f), primaryTransform.scale);
	glm::mat4 deltaMatrix(1.0f);

	if (ImGuizmo::Manipulate(cameraView, cameraProjection, manipulationSettings.currentGizmoOperation, manipulationSettings.relativeSpaceMode, &matrix[0][0], &deltaMatrix[0][0], manipulationSettings.snap ? &snap.x : nullptr)) {
		if (manipulationSettings.currentGizmoOperation == ImGuizmo::ROTATE) {
			glm::quat deltaRotation = glm::quat_cast(glm::mat3(deltaMatrix));
			primaryTransform.rotation = glm::normalize(deltaRotation * primaryTransform.rotation);

			for (auto &secondaryItem : secondaryItems) {
				if (secondaryItem.index() == 0 && activeScene->getEntity(std::get<entt::entity>(secondaryItem)).hasComponent<TransformComponent>()) {
					TransformComponent &secondaryTransform = activeScene->getRegistry().get<TransformComponent>(std::get<entt::entity>(secondaryItem));
					secondaryTransform.rotation = glm::normalize(deltaRotation * secondaryTransform.rotation);
				}
			}
		} else {
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::quat orientation;
			glm::mat4 localMatrix = glm::inverse(globalParentMatrix) * matrix;
			glm::decompose(localMatrix, primaryTransform.scale, orientation, primaryTransform.position, skew, perspective);

			glm::vec3 deltaPosition = glm::vec3(0.0);
			glm::vec3 deltaScale = glm::vec3(1.0f);
			glm::decompose(deltaMatrix, deltaScale, orientation, deltaPosition, skew, perspective);

			for (auto &secondaryItem : secondaryItems) {
				if (secondaryItem.index() == 0 && activeScene->getEntity(std::get<entt::entity>(secondaryItem)).hasComponent<TransformComponent>()) {
					TransformComponent &secondaryTransform = activeScene->getRegistry().get<TransformComponent>(std::get<entt::entity>(secondaryItem));
					secondaryTransform.position += deltaPosition;
					secondaryTransform.scale *= deltaScale;
				}
			}
		}
	}
}

bool ViewPanel::mouseSelectEvent(Event &e) {
	MouseButtonPressedEvent &event = static_cast<MouseButtonPressedEvent &>(e);
	if (event.getButton() == SDL_BUTTON_LEFT && event.getClicks() == 2) {
		AppContext context = Editor::get().getContext();

		PerspectiveView &view = Editor::get().editorView;

		glm::vec2 mousePos = Editor::get().getLayer<CitronInput::InputLayer>()->getMousePosition();
		glm::vec2 mousePosInViewport = glm::vec2(mousePos.x - viewportPos.x, mousePos.y - viewportPos.y);
		glm::vec2 mouseViewportUV = glm::vec2(mousePosInViewport.x / viewportSize.x, mousePosInViewport.y / viewportSize.y);

		CITRON_CLIENT_INFO("MX {} MY {}", mousePos.x / (viewportPos.x + viewportSize.x), mousePos.y / (viewportPos.y + viewportSize.y));
		CITRON_CLIENT_INFO("FOV {} ASPECT {}", view.fov, view.aspect);

		float halfVertical = std::tan(glm::radians(view.fov) * 0.5f);
		float halfHorizontal = halfVertical * view.aspect;
		glm::vec3 viewportRight = glm::normalize(glm::cross(view.forward, view.up));
		glm::vec3 mouseRayWorldPos = view.position;
		float mouseXFactor = mouseViewportUV.x * 2.0f - 1.0f;
		float mouseYFactor = mouseViewportUV.y * 2.0f - 1.0f;
		glm::vec3 mouseRayWorldDir = view.forward + view.up * halfVertical * -mouseYFactor + viewportRight * mouseXFactor * halfHorizontal;

		bool selected = false;

		float lastDistance = std::numeric_limits<float>::max();
		entt::entity lastClosestEntity = entt::null;

		for (entt::entity entity : Editor::get().getContext().sceneManager.getActiveScene()->getRegistry().view<MeshComponent>()) {
			MeshComponent &meshComponent = context.sceneManager.getActiveScene()->getRegistry().get<MeshComponent>(entity);
			std::shared_ptr<Mesh> mesh = context.assetManager.getAsset<Mesh>(meshComponent.meshAsset.uuid);
			glm::mat4 transform = context.sceneManager.getActiveScene()->getGlobalTransform(entity);
			glm::vec3 minBBPos = glm::xyz(transform * glm::vec4(mesh->getBoundsMin(), 1.0f));
			glm::vec3 maxBBPos = glm::xyz(transform * glm::vec4(mesh->getBoundsMax(), 1.0f));

			if (MathUtils::rayIntersectsAABB(mouseRayWorldPos, mouseRayWorldDir, minBBPos, maxBBPos)) {
				float distance = glm::distance(mouseRayWorldPos, minBBPos);
				if (distance < lastDistance) {
					lastDistance = distance;
					lastClosestEntity = entity;
					selected = true;
				}
			}
		}

		if (!selected) {
			Editor::get().getEditorContext().setCurrentlySelectedItem(nullptr);
		} else {
			Editor::get().getEditorContext().setCurrentlySelectedItem(lastClosestEntity, App::get().getLayer<CitronInput::InputLayer>()->isPressed(SDLK_LSHIFT) || App::get().getLayer<CitronInput::InputLayer>()->isPressed(SDLK_LCTRL));
		}
	}

	return false;
}
