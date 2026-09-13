#include "game_panel.hpp"
#include "editor.hpp"

void GamePanel::onAttach() {}
void GamePanel::onDetach() {}
void GamePanel::onUpdate() {}
void GamePanel::onDraw() {
	ImGui::Begin("Game", nullptr);
	if (shouldBeFocused) {
		ImGui::SetWindowFocus();
		shouldBeFocused = false;
	}

	viewportSize = ImGui::GetContentRegionAvail();
	ImVec2 viewportPos = ImGui::GetCursorScreenPos();
	WGPUTextureView view = sceneView;
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((ImTextureID)(uintptr_t)view, viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y));
	ImGui::End();
}
void GamePanel::onEvent(Event &e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<EditorPlaymodeEvent>(CITRON_BIND_EVENT_FN(GamePanel::onPlaymodeEvent));
}

bool GamePanel::onPlaymodeEvent(Event &e) {
	EditorPlaymodeEvent &playmodeEvent = static_cast<EditorPlaymodeEvent &>(e);
	if (playmodeEvent.state == EditorPlaymodeState::Playing) {
		shouldBeFocused = true;
	}

	return false;
}
