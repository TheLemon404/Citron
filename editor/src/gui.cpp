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
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

void GuiLayer::onAttach() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigInputTrickleEventQueue = false;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
}

void GuiLayer::onDetach() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplWGPU_Shutdown();
	ImGui::DestroyContext();
}

void GuiLayer::onUpdate() {}

void GuiLayer::drawGui(wgpu::TextureView &sceneView,
					   CitronGraphics::RenderPass &currentRenderPass) {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport();

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

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(),
								  currentRenderPass.getRenderPassEncoder());
}

void GuiLayer::onEvent(Event &e) {
	if (SDL_Event *sdlEvent = (SDL_Event *)e.getInternalEvent())
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
}
