#include "gui.hpp"
#include "app.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_wgpu.h"
#include "device.hpp"
#include "editor.hpp"
#include "graphics.hpp"
#include <imgui.h>

void GuiLayer::onAttach() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();

	App &editorApp = Editor::get();
	ImGui_ImplSDL3_InitForOther(
		(SDL_Window *)editorApp.getWindow().getSDLWindow());
	ImGui_ImplWGPU_InitInfo initInfo = {};
	initInfo.Device =
		(WGPUDevice)editorApp.getGraphicsContext().getDevice().getWGPUDevice();
	initInfo.NumFramesInFlight = 3;
	initInfo.RenderTargetFormat =
		(WGPUTextureFormat)editorApp.getGraphicsContext()
			.getDevice()
			.getWGPUPreferredSurfaceFormat();
	ImGui_ImplWGPU_Init(&initInfo);
}

void GuiLayer::onDetach() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplWGPU_Shutdown();
	ImGui::DestroyContext();
}

void GuiLayer::onUpdate() {}

void GuiLayer::onRender() {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(),
								  Editor::get()
									  .getGraphicsContext()
									  .getDevice()
									  .getWGPURenderPassEncoder());
}

void GuiLayer::onEvent(Event &e) {}
