#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "gui.hpp"
#include "app.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_wgpu.h"
#include "device.hpp"
#include "editor.hpp"
#include "event.hpp"
#include "keyboard.hpp"
#include "panel.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include <IconsFontAwesome6.h>
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

GuiLayer::GuiLayer(AppContext appContext) : Layer("GuiLayer"),
											viewPanel(appContext, Editor::get().getEditorContext().getCurrentlySelectedItem(), Editor::get().editorView),
											appContext(appContext),
											assetPropertiesPanel(appContext),
											assetPanel(appContext, assetPropertiesPanel),
											assetRegistryPanel(appContext),
											outlinerPanel(appContext),
											consolePanel(appContext),
											inspectorPanel(appContext) {}

void GuiLayer::onAttach() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigInputTrickleEventQueue = false;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	const std::string citronFont =
		std::string(CITRON_PROGRAM_FOLDER) +
		"/EngineResources/Fonts/JetBrainsMono-Light.ttf";
	io.Fonts->AddFontFromFileTTF(citronFont.c_str());
	float iconFontSize =
		16 * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced
						  // by 2.0f/3.0f in order to align correctly

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	std::string fontPath = std::string(CITRON_PROGRAM_FOLDER) +
						   "/EngineResources/Fonts/" + FONT_ICON_FILE_NAME_FAS;
	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), iconFontSize, &icons_config,
								 icons_ranges);

	App &editorApp = Editor::get();
	ImGui_ImplSDL3_InitForOther(
		(SDL_Window *)editorApp.getContext().window.getSDLWindow());
	ImGui_ImplWGPU_InitInfo initInfo = {};
	initInfo.Device =
		(WGPUDevice)editorApp.getContext().renderer.getContext().device.getWGPUDevice();
	initInfo.NumFramesInFlight = 2;
	initInfo.RenderTargetFormat = (WGPUTextureFormat)editorApp.getContext().renderer.getContext().device.getWGPUPreferredSurfaceFormat();
	initInfo.DepthStencilFormat = WGPUTextureFormat_Undefined;
	ImGui_ImplWGPU_Init(&initInfo);

	appContext.renderer.onGuiDrawCallback = CITRON_BIND_FN(
		GuiLayer::drawGui, std::placeholders::_1, std::placeholders::_2);

	applyTheme();

	viewPanel.onAttach();
	assetPanel.onAttach();
	assetPropertiesPanel.onAttach();
	outlinerPanel.onAttach();
	consolePanel.onAttach();
	inspectorPanel.onAttach();
	assetRegistryPanel.onAttach();
}

void GuiLayer::onDetach() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplWGPU_Shutdown();
	ImGui::DestroyContext();

	viewPanel.onDetach();
	assetPanel.onDetach();
	assetPropertiesPanel.onDetach();
	outlinerPanel.onDetach();
	consolePanel.onDetach();
	inspectorPanel.onDetach();
	assetRegistryPanel.onDetach();
}

void GuiLayer::onUpdate() {
	viewPanel.onUpdate();
	assetPanel.onUpdate();
	assetPropertiesPanel.onUpdate();
	outlinerPanel.onUpdate();
	consolePanel.onUpdate();
	inspectorPanel.onUpdate();
	assetRegistryPanel.onUpdate();
}

void GuiLayer::drawGui(wgpu::TextureView &sceneView,
					   CitronGraphics::RenderPass &currentRenderPass) {
	EditorContext &context = Editor::get().getEditorContext();

	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGui::DockSpaceOverViewport();

	bool openRenameScenePopup = false;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Project")) {
				std::string projectPath = CitronIO::IO::openFileDialog(
					"Project", CITRON_PROJECT_FILE_ENDING);
				if (!projectPath.empty()) {
					Editor::get().openProject(projectPath);

					assetPanel.currentDirectory =
						context.projectFilePath.parent_path();
					assetPanel.pendingRefreshDirectory = true;
				}
			}
			if (ImGui::MenuItem("Create Scene")) {
				Editor::get().createScene();
			}
			if (ImGui::MenuItem("Open Scene")) {
				std::string scenePath = CitronIO::IO::openFileDialog(
					"Scene", CITRON_SCENE_FILE_ENDING);
				if (!scenePath.empty()) {
					Editor::get().openScene(scenePath);

					assetPanel.pendingRefreshDirectory = true;
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Rename Current Scene")) {
				ImGui::OpenPopup("SceneRenamePopup");
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Asset Registry")) {
				assetRegistryPanel.showWindow = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (openRenameScenePopup) {
		ImGui::OpenPopup("SceneRenamePopup");
	}

	std::string newName;
	if (ImGui::BeginPopup("SceneRenamePopup")) {
		if (ImGui::InputTextWithHint("Rename Scene", "Scene Name", &newName,
									 ImGuiInputTextFlags_EnterReturnsTrue)) {

			appContext.sceneManager.getActiveScene()->rename(newName);
			ImGui::CloseCurrentPopup();

			CITRON_CORE_INFO("Renamed scene to {}", newName);
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	viewPanel.setView(sceneView);
	viewPanel.onDraw();
	assetPanel.onDraw();
	assetPropertiesPanel.onDraw();
	outlinerPanel.onDraw();
	consolePanel.onDraw();
	inspectorPanel.onDraw();
	assetRegistryPanel.onDraw();

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(),
								  currentRenderPass.getRenderPassEncoder());
}

void GuiLayer::onEvent(Event &e) {
	if (e.isInCategory(CitronCore::EventCategoryInput)) {
		if (e.getEventType() == EventType::KeyJustPressed) {
			KeyJustPressedEvent &event = static_cast<KeyJustPressedEvent &>(e);
			if (event.getKeycode() == SDLK_S && event.getMods() & SDLK_LCTRL) {
				CITRON_CORE_INFO("Serializing assets...");

				appContext.assetManager.serializeAssets();
				Editor::get().saveCurrentScene();
			}
		}
	}

	viewPanel.onEvent(e);
	assetPanel.onEvent(e);
	outlinerPanel.onEvent(e);
	consolePanel.onEvent(e);
	inspectorPanel.onEvent(e);
	assetRegistryPanel.onEvent(e);
	if (SDL_Event *sdlEvent = (SDL_Event *)e.getInternalEvent())
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
}

void GuiLayer::applyTheme() {
	ImGuiStyle &style = ImGui::GetStyle();

	style.WindowMenuButtonPosition = ImGuiDir_None;

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(2.0f, 2.0f);
	style.WindowRounding = 3.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.6f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 3.0f;
	style.ChildBorderSize = 0.0f;
	style.PopupRounding = 3.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(4.0f, 4.0f);
	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 4.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 2.5f;
	style.TabRounding = 3.0f;
	style.TabBorderSize = 0.0f;
	style.TabCloseButtonMinWidthSelected = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	style.DockingSeparatorSize = 2.0f;
	style.MenuItemRounding = 3.0f;

	style.FontSizeBase = 16.0f;
	style.TreeLinesSize = 1.0f;
	style.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesFull;

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] =
		ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] =
		ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] =
		ImVec4(0.12875539f, 0.1287541f, 0.1287541f, 1.0f);
	style.Colors[ImGuiCol_Border] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] =
		ImVec4(0.10729611f, 0.10729504f, 0.10729504f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] =
		ImVec4(0.25490198f, 0.25490198f, 0.25490198f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] =
		ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] =
		ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] =
		ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] =
		ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] =
		themeColor;
	style.Colors[ImGuiCol_SliderGrab] =
		themeColor;
	style.Colors[ImGuiCol_SliderGrabActive] =
		themeColor;
	style.Colors[ImGuiCol_Button] =
		ImVec4(0.32941177f, 0.32941177f, 0.32941177f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] =
		ImVec4(0.35193133f, 0.35192782f, 0.35192782f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] =
		ImVec4(0.20171672f, 0.2017147f, 0.2017147f, 1.0f);
	style.Colors[ImGuiCol_Header] =
		ImVec4(0.32941177f, 0.32941177f, 0.32941177f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] =
		ImVec4(0.32941177f, 0.32941177f, 0.32941177f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] =
		ImVec4(0.32941177f, 0.32941177f, 0.32941177f, 1.0f);
	style.Colors[ImGuiCol_Separator] =
		ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
	style.Colors[ImGuiCol_SeparatorHovered] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] =
		ImVec4(0.3862661f, 0.38626224f, 0.38626224f, 1.0f);
	style.Colors[ImGuiCol_Tab] =
		ImVec4(0.13304722f, 0.1330459f, 0.1330459f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] =
		ImVec4(0.13304722f, 0.1330459f, 0.1330459f, 1.0f);
	style.Colors[ImGuiCol_TabActive] =
		ImVec4(0.15879828f, 0.15879668f, 0.15879668f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] =
		ImVec4(0.13304722f, 0.1330459f, 0.1330459f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] =
		ImVec4(0.15879828f, 0.15879668f, 0.15879668f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] =
		ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] =
		ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] =
		themeColor;
	style.Colors[ImGuiCol_PlotHistogramHovered] =
		themeColor;
	style.Colors[ImGuiCol_TableHeaderBg] =
		ImVec4(0.32941177f, 0.32941177f, 0.32941177f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] =
		ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] =
		ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] =
		ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] =
		ImVec4(1.0f, 0.51502144f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] =
		ImVec4(0.38431373f, 0.38431373f, 0.38431373f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] =
		ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);

	style.Colors[ImGuiCol_CheckboxSelectedBg] =
		themeSecondaryColor;
	style.Colors[ImGuiCol_DockingPreview] =
		themeColor;
	style.Colors[ImGuiCol_DockingEmptyBg] =
		ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_TabSelectedOverline] = themeColor;

	ImGuizmo::Style &guizmoStyle = ImGuizmo::GetStyle();
	guizmoStyle.Colors[ImGuizmo::COLOR::DIRECTION_X] = xColor;
	guizmoStyle.Colors[ImGuizmo::COLOR::DIRECTION_Y] = yColor;
	guizmoStyle.Colors[ImGuizmo::COLOR::DIRECTION_Z] = zColor;
	guizmoStyle.Colors[ImGuizmo::COLOR::PLANE_X] = xColor;
	guizmoStyle.Colors[ImGuizmo::COLOR::PLANE_Y] = yColor;
	guizmoStyle.Colors[ImGuizmo::COLOR::PLANE_Z] = zColor;

	guizmoStyle.RotationLineThickness = 3.0f;
	guizmoStyle.RotationOuterLineThickness = 3.0f;
	guizmoStyle.HatchedAxisLineThickness = 3.0f;
	guizmoStyle.ScaleLineThickness = 3.0f;
	guizmoStyle.ScaleLineCircleSize = 6.0f;
	guizmoStyle.TranslationLineArrowSize = 6.0f;
}
