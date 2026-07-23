#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "event.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include "logger.hpp"
#include <input.hpp>
#include <io.hpp>
#include <serialization.hpp>
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
	YAML::Node projectFileNode = YAML::LoadFile(editorContext.projectFilePath);
	if (projectFileNode["last_scene"].IsDefined() &&
		!projectFileNode["last_scene"].IsNull()) {
		std::string editedScenePath =
			projectFileNode["last_scene"].as<std::string>();

		if (!CitronIO::IO::fileExists(editedScenePath)) {
			editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
			return;
		}

		std::string sceneName =
			editedScenePath.substr(editedScenePath.find_last_of('/') + 1,
								   editedScenePath.find_last_of('.'));

		FileStreamReader reader = FileStreamReader(editedScenePath);
		editorContext.setCurrentScene(std::make_shared<Scene>(sceneName));
		editorContext.getCurrentScene()->deserialize(reader);
		editorContext.currentlyEditedSceneAssetPath = editedScenePath;
	} else {
		editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
	}
}

void EditorLayer::onUpdate() {}

void EditorLayer::onDetach() {
	if (!editorContext.projectFilePath.empty()) {
		CitronIO::IO::writeFile(
			std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml",
			"last_project: " + editorContext.projectFilePath);

		if (!editorContext.currentlyEditedSceneAssetPath.empty()) {
			YAML::Node projectFileNode =
				YAML::LoadFile(editorContext.projectFilePath);
			projectFileNode["last_scene"] =
				editorContext.currentlyEditedSceneAssetPath;
			CitronIO::IO::writeFile(editorContext.projectFilePath,
									YAML::Dump(projectFileNode));

			saveCurrentScene();
		}
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
	std::string newProjectPath = CitronIO::IO::saveFileDialog(
		"Project", CITRON_PROJECT_FILE_ENDING, nullptr, 0);
	if (!newProjectPath.empty()) {
		CitronIO::IO::writeFile(
			newProjectPath,
			"name: '" +
				newProjectPath.substr(newProjectPath.find_last_of("\\") + 1) +
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
	editorContext.projectRootFolderPath =
		projectFilePath.substr(0, projectFilePath.find_last_of("\\"));
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
	FileStreamWriter fwriter =
		FileStreamWriter(editorContext.currentlyEditedSceneAssetPath);

	if (editorContext.currentlyEditedSceneAssetPath.empty()) {
		editorContext.currentlyEditedSceneAssetPath =
			CitronIO::IO::saveFileDialog("Scene", CITRON_SCENE_FILE_ENDING,
										 nullptr, 0);
		if (editorContext.currentlyEditedSceneAssetPath.empty()) {
			CITRON_CLIENT_WARN("Scene: {} was not saved",
							   editorContext.getCurrentScene()->getName());
			return;
		}
		CitronIO::IO::createFile(editorContext.currentlyEditedSceneAssetPath);
	}

	editorContext.getCurrentScene()->serialize(fwriter);

	CITRON_CLIENT_INFO(
		"Scene: {} saved to {}: ", editorContext.getCurrentScene()->getName(),
		editorContext.currentlyEditedSceneAssetPath);
}

void Editor::onPushClientLayers() {
	pushLayer<EditorLayer>();
	pushLayer<GuiLayer>();
}
