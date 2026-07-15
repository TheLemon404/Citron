#include "editor.hpp"
#include "logger.hpp"

void EditorLayer::onAttach() {}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {}

void EditorLayer::onEvent(CitronCore::Event &e) {
  CITRON_CLIENT_INFO(e.toString());
}

Editor::Editor() : CitronCore::App() { pushLayer(new EditorLayer()); }
