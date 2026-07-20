#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "event.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include <input.hpp>
#include <io.hpp>

void EditorLayer::onAttach() {
	// TODO: attempt to load last edited scene
	editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {}

void EditorLayer::onEvent(CitronCore::Event &e) {
	if (e.isInCategory(CitronCore::EventCategoryInput)) {
		if (e.getEventType() == EventType::KeyJustPressed) {
			KeyJustPressedEvent &event = static_cast<KeyJustPressedEvent &>(e);
			if (event.getKeycode() == SDLK_S && event.getMods() & SDLK_LCTRL) {
				saveCurrentScene();
			}
		}
	}
}

void EditorLayer::saveCurrentScene() {
	if (editorContext.sceneSavePath.empty()) {
		editorContext.sceneSavePath =
			CitronIO::IO::saveFileDialog("Scene", "Scene", "txt", nullptr, 0);
		if (!editorContext.sceneSavePath.empty()) {
			editorContext.getCurrentScene()->save(editorContext.sceneSavePath);
			CITRON_CLIENT_WARN("Scene: {} saved to {}: ",
							   editorContext.getCurrentScene()->getName(),
							   editorContext.sceneSavePath);
		}
	} else {
		editorContext.getCurrentScene()->save(editorContext.sceneSavePath);
	}
}

void Editor::onPushClientLayers() {
	pushLayer<GuiLayer>();
	pushLayer<EditorLayer>();
}
