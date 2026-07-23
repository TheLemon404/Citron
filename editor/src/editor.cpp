#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "event.hpp"
#include "gui.hpp"
#include "keyboard.hpp"
#include "logger.hpp"
#include "panel.hpp"
#include "yaml-cpp/node/emit.h"
#include <input.hpp>
#include <io.hpp>
#include <memory>
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
	} catch (const std::exception &e) {
		CITRON_CORE_CRITICAL("Error: {}", e.what());
		CITRON_CORE_ERROR(
			"Failed to load last project: {}. Creating new project...",
			editorContext.projectFilePath);
		createProject();
	}
}

void EditorLayer::onAttach() {
	YAML::Node projectFileNode = YAML::LoadFile(editorContext.projectFilePath);
	if (projectFileNode["last_scene"].IsDefined() &&
		!projectFileNode["last_scene"].IsNull()) {
		std::string lastEditedSceneFile =
			projectFileNode["last_scene"].as<std::string>();

		if (!openSceneFile(lastEditedSceneFile)) {
			CITRON_CLIENT_ERROR("Failed to load last edited scene file: {}",
								lastEditedSceneFile);
			editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
		}

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

bool EditorLayer::openSceneFile(std::string sceneAssetPath) {
	CITRON_CLIENT_INFO("Opening scene file: " + sceneAssetPath);

	if (!CitronIO::IO::fileExists(sceneAssetPath))
		return false;

	FileStreamReader reader = FileStreamReader(sceneAssetPath);
	editorContext.setCurrentScene(std::make_shared<Scene>(""));
	editorContext.getCurrentScene()->deserialize(reader);
	editorContext.currentlyEditedSceneAssetPath = sceneAssetPath;

	return true;
}

std::string EditorLayer::createSceneFile() {
	std::string newSceneFile = CitronIO::IO::saveFileDialog(
		"Scene", CITRON_SCENE_FILE_ENDING, nullptr, 0);
	if (newSceneFile.empty()) {
		CITRON_CLIENT_WARN("Scene: {} was not saved",
						   editorContext.getCurrentScene()->getName());
		return "";
	}
	CitronIO::IO::createFile(newSceneFile);
	return newSceneFile;
}

bool EditorLayer::createProject() {
	CITRON_CLIENT_INFO("Creating project...");
	std::string newProjectPath = CitronIO::IO::saveFileDialog(
		"Project", CITRON_PROJECT_FILE_ENDING, nullptr, 0);
	if (!newProjectPath.empty()) {
		YAML::Node node = YAML::Node();
		node["name"] =
			newProjectPath.substr(newProjectPath.find_last_of("\\") + 1,
								  newProjectPath.find_last_of("."));
		CitronIO::IO::createFile(newProjectPath);
		CitronIO::IO::writeFile(newProjectPath, YAML::Dump(node));
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

	editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
	editorContext.currentlyEditedSceneAssetPath = "";

	if (node["last_scene"].IsDefined() && !node["last_scene"].IsNull() &&
		!node["last_scene"].as<std::string>().empty()) {
		editorContext.currentlyEditedSceneAssetPath =
			node["last_scene"].as<std::string>();
		FileStreamReader reader(editorContext.currentlyEditedSceneAssetPath);
		editorContext.getCurrentScene()->deserialize(reader);
	}

	std::string editorTitle =
		std::string("Citron Editor: ") + editorContext.projectName;
	Editor::get().getWindow().setName(editorTitle);

	YAML::Node citronConfig =
		YAML::LoadFile(std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml");
	citronConfig["last_project"] = projectFilePath;
	CitronIO::IO::writeFile(std::string(CITRON_PROGRAM_FOLDER) + "/citron.yaml",
							YAML::Dump(citronConfig));

	CITRON_CLIENT_INFO("Opened project: {}", projectFilePath);
	return true;
}

void EditorLayer::saveCurrentScene() {
	if (editorContext.currentlyEditedSceneAssetPath.empty() ||
		!CitronIO::IO::fileExists(
			editorContext.currentlyEditedSceneAssetPath)) {
		editorContext.currentlyEditedSceneAssetPath = createSceneFile();
	}

	FileStreamWriter fwriter =
		FileStreamWriter(editorContext.currentlyEditedSceneAssetPath);
	editorContext.getCurrentScene()->serialize(fwriter);

	CITRON_CLIENT_INFO(
		"Scene: {} saved to {}: ", editorContext.getCurrentScene()->getName(),
		editorContext.currentlyEditedSceneAssetPath);
}

void Editor::onPushClientLayers() {
	pushLayer<EditorLayer>();
	pushLayer<GuiLayer>();
}
