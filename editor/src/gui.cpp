#include "gui.hpp"
#include "app.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_wgpu.h"
#include "device.hpp"
#include "editor.hpp"
#include "event.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include <cstdint>
#include <imgui.h>
#include <io.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

void GuiLayer::onAttach() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigInputTrickleEventQueue = false;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->AddFontFromFileTTF(CITRON_INIT_FONT);
	App &editorApp = Editor::get();
	ImGui_ImplSDL3_InitForOther(
		(SDL_Window *)editorApp.getWindow().getSDLWindow());
	ImGui_ImplWGPU_InitInfo initInfo = {};
	initInfo.Device =
		(WGPUDevice)editorApp.getRenderer().getDevice().getWGPUDevice();
	initInfo.NumFramesInFlight = 2;
	initInfo.RenderTargetFormat = (WGPUTextureFormat)editorApp.getRenderer()
									  .getDevice()
									  .getWGPUPreferredSurfaceFormat();
	ImGui_ImplWGPU_Init(&initInfo);

	Editor::get().getRenderer().onGuiDrawCallback = std::bind(
		&GuiLayer::drawGui, this, std::placeholders::_1, std::placeholders::_2);

	applyTheme();

	consolePanel.onAttach();
}

void GuiLayer::onDetach() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplWGPU_Shutdown();
	ImGui::DestroyContext();

	consolePanel.onDetach();
}

void GuiLayer::onUpdate() { consolePanel.onUpdate(); }

void GuiLayer::drawGui(wgpu::TextureView &sceneView,
					   CitronGraphics::RenderPass &currentRenderPass) {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport();
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Create New Project")) {
				std::string newProjectPath = CitronIO::IO::saveFileDialog(
					"Project", "ctrnproject", nullptr, 0);
				CitronIO::IO::createFile(newProjectPath);
				Editor::get()
					.getLayerStack()
					.getLayer<EditorLayer>()
					->openProject(newProjectPath);
			}
			if (ImGui::MenuItem("Open Project")) {
				Editor::get()
					.getLayerStack()
					.getLayer<EditorLayer>()
					->openProject(
						CitronIO::IO::openFileDialog("Project", "ctrnproject"));
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGui::Begin("Viewport");
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	WGPUTextureView view = sceneView;
	ImGui::Image((ImTextureID)(uintptr_t)view, viewportSize);
	ImGui::End();
	ImGui::Begin("Scene");
	ImGui::End();
	ImGui::Begin("Inspector");
	ImGui::End();
	ImGui::Begin("Assets");
	ImGui::End();

	consolePanel.onDraw();

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(),
								  currentRenderPass.getRenderPassEncoder());
}

void GuiLayer::onEvent(Event &e) {
	if (SDL_Event *sdlEvent = (SDL_Event *)e.getInternalEvent())
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
}

void GuiLayer::applyTheme() {
	ImGuiStyle &style = ImGui::GetStyle();

	style.WindowMenuButtonPosition = ImGuiDir_None;

	style.TabBarBorderSize = 1.0f;
	style.TabRounding = 3.0f;
	style.TabBarOverlineSize = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowPadding = ImVec2(3.0, 3.0);
	style.FramePadding = ImVec2(3.0, 3.0);
	style.ItemSpacing = ImVec2(5.0, 3.0);
	style.ItemInnerSpacing = ImVec2(5.0, 3.0);
	style.ScrollbarSize = 13.0f;
	style.ScrollbarRounding = 12.0f;
	style.ScrollbarPadding = 3.0f;
	style.DockingSeparatorSize = 1.0f;
	style.FontSizeBase = 16.0f;
	style.FrameRounding = 3.0f;

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] =
		ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] =
		ImVec4(0.060085833f, 0.06008523f, 0.06008523f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] =
		ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
	style.Colors[ImGuiCol_Border] =
		ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] =
		ImVec4(0.20171672f, 0.2017147f, 0.2017147f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] =
		ImVec4(0.25490198f, 0.25490198f, 0.25490198f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] =
		ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] =
		ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] =
		ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] =
		ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] =
		ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] =
		ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] =
		ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.5665236f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.5647028f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] =
		ImVec4(1.0f, 0.43776822f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Button] =
		ImVec4(0.25490198f, 0.25490198f, 0.25490198f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] =
		ImVec4(0.35193133f, 0.35192782f, 0.35192782f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] =
		ImVec4(0.20171672f, 0.2017147f, 0.2017147f, 1.0f);
	style.Colors[ImGuiCol_Header] =
		ImVec4(0.25490198f, 0.25490198f, 0.25490198f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] =
		ImVec4(0.25490198f, 0.25490198f, 0.25490198f, 1.0f);
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
		ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] =
		ImVec4(0.33476394f, 0.3347606f, 0.3347606f, 1.0f);
	style.Colors[ImGuiCol_TabActive] =
		ImVec4(0.2532189f, 0.25321636f, 0.25321636f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] =
		ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] =
		ImVec4(0.15879828f, 0.15879668f, 0.15879668f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] =
		ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] =
		ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.5647059f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] =
		ImVec4(1.0f, 0.5647059f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] =
		ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] =
		ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] =
		ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.005f);
	style.Colors[ImGuiCol_TextSelectedBg] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] =
		ImVec4(0.38431373f, 0.38431373f, 0.38431373f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] =
		ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);

	style.Colors[ImGuiCol_CheckboxSelectedBg] =
		ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 1.0f);
}
