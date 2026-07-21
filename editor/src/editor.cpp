#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "event.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include "logger.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include <input.hpp>
#include <io.hpp>
#include <yaml-cpp/yaml.h>

EditorLayer::EditorLayer() : CitronCore::Layer("EditorLayer") {
	YAML::Node configNode =
		YAML::LoadFile(std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml");

	if (!configNode["last_project"].IsNull()) {
		editorContext.projectFilePath =
			configNode["last_project"].as<std::string>();
	} else {
		CITRON_CORE_WARN(
			"No last opened project found... creating new project...");
		while (!createProject()) {
		}
	}

	try {
		openProject(editorContext.projectFilePath);
	} catch (...) {
		CITRON_CORE_ERROR(
			"Failed to load last project. Creating new project...");
		while (!createProject()) {
		}
	}
}

void EditorLayer::onAttach() {
	// TODO: attempt to load last edited scene
	editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {
	if (!editorContext.projectFilePath.empty()) {
		CitronIO::IO::writeFile(
			std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml",
			"last_project: " + editorContext.projectFilePath);
		CITRON_CORE_ERROR(editorContext.projectFilePath);
	}
}

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

bool EditorLayer::createProject() {
	CITRON_CLIENT_INFO("Creating project...");
	std::string newProjectPath =
		CitronIO::IO::saveFileDialog("Project", "ctrnproject", nullptr, 0);
	if (!newProjectPath.empty()) {
		CitronIO::IO::writeFile(
			newProjectPath,
			"name: '" +
				newProjectPath.substr(newProjectPath.find_last_of("/") + 1) +
				"'");
		openProject(newProjectPath);
		return true;
	}
	return false;
}

bool EditorLayer::openProject(std::string projectFilePath) {
	CITRON_CORE_ASSERT(!projectFilePath.empty(), "projectFilePath is empty");
	if (projectFilePath.empty())
		return false;
	editorContext.projectFilePath = projectFilePath;
	YAML::Node node = YAML::LoadFile(projectFilePath);
	editorContext.projectName = node["name"].as<std::string>();
	std::string editorTitle =
		std::string("Citron Editor: ") + editorContext.projectName;
	Editor::get().getWindow().setName(editorTitle);
	CitronIO::IO::writeFile(std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml",
							"last_project: " + projectFilePath);
	CITRON_CLIENT_INFO("Opened project: {}", projectFilePath);
	return true;
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
