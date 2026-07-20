#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "event.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include <input.hpp>
#include <io.hpp>
#include <yaml-cpp/yaml.h>

constexpr const char *CITRON_INIT_FILE =
	"C:/Users/vghy7/OneDrive/Desktop/Citron/citron.yaml";

EditorContext::EditorContext(std::string projectFilePath)
	: projectFilePath(projectFilePath) {}

EditorLayer::EditorLayer()
	: CitronCore::Layer("EditorLayer"),
	  editorContext(YAML::LoadFile(CITRON_INIT_FILE)["last_project"].IsNull()
						? std::string()
						: YAML::LoadFile(CITRON_INIT_FILE)["last_project"]
							  .as<std::string>()) {
	if (editorContext.projectFilePath.empty()) {
		openProject(CitronIO::IO::openFileDialog("Project", "ctrnproject"));
	} else {
		openProject(editorContext.projectFilePath);
	}
}

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

void EditorLayer::openProject(std::string projectFilePath) {
	editorContext.projectFilePath = projectFilePath;
	CitronIO::IO::writeFile(CITRON_INIT_FILE,
							"last_project: " + projectFilePath);
	CITRON_CLIENT_INFO("Opened project: {}", projectFilePath);
}

void EditorLayer::saveCurrentScene() {
	if (editorContext.sceneSaveAssetPath.empty()) {
		editorContext.sceneSaveAssetPath =
			CitronIO::IO::saveFileDialog("Scene", "scene", nullptr, 0);
		if (!editorContext.sceneSaveAssetPath.empty()) {
			editorContext.getCurrentScene()->save(
				editorContext.sceneSaveAssetPath);
			CITRON_CLIENT_INFO("Scene: {} saved to {}: ",
							   editorContext.getCurrentScene()->getName(),
							   editorContext.sceneSaveAssetPath);
		}
	} else {
		editorContext.getCurrentScene()->save(editorContext.sceneSaveAssetPath);
	}
}

void Editor::onPushClientLayers() {
	pushLayer<GuiLayer>();
	pushLayer<EditorLayer>();
}
