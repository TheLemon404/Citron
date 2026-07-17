#include "editor.hpp"
#include "gui.hpp"

void EditorLayer::onAttach() {}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {}

void EditorLayer::onEvent(CitronCore::Event &e) {}

void Editor::onPushClientLayers() {
	pushLayer<GuiLayer>();
	pushLayer<EditorLayer>();
}
