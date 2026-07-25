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

Editor::Editor(const std::string &projectFilePath)
	: CitronCore::App(
		  false, std::make_unique<EditorAssetManager>(projectFilePath.substr(
					 0, projectFilePath.find_last_of('\\') + 1))) {
	editorContext.projectFilePath = projectFilePath;
}

void Editor::init() {
	App::init();

	openProject(editorContext.projectFilePath);

	assetManager->initializeAssetRegistry();

	pushLayer<GuiLayer>();

	YAML::Node projectFileNode = YAML::LoadFile(editorContext.projectFilePath);
	if (projectFileNode["last_scene"].IsDefined() &&
		!projectFileNode["last_scene"].IsNull()) {
		std::string lastEditedSceneFile =
			projectFileNode["last_scene"].as<std::string>();

		if (!openScene(lastEditedSceneFile)) {
			CITRON_CLIENT_ERROR("Failed to load last edited scene file: {}",
								lastEditedSceneFile);
			editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
		}

	} else {
		editorContext.setCurrentScene(std::make_shared<Scene>("Scene"));
	}
}

void Editor::update() { App::update(); }

void Editor::close() {
	App::close();
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

void Editor::onEvent(CitronCore::Event &e) {
	App::onEvent(e);
	if (e.isInCategory(CitronCore::EventCategoryInput)) {
		if (e.getEventType() == EventType::KeyJustPressed) {
			KeyJustPressedEvent &event = static_cast<KeyJustPressedEvent &>(e);
			if (event.getKeycode() == SDLK_S && event.getMods() & SDLK_LCTRL) {
				saveCurrentScene();
			}
		}
	}
}

bool Editor::openScene(std::string sceneAssetPath) {
	CITRON_CLIENT_INFO("Opening scene file: " + sceneAssetPath);

	if (!CitronIO::IO::fileExists(sceneAssetPath))
		return false;

	FileStreamReader reader = FileStreamReader(sceneAssetPath);
	editorContext.setCurrentScene(std::make_shared<Scene>(""));
	editorContext.getCurrentScene()->deserialize(reader);
	editorContext.currentlyEditedSceneAssetPath = sceneAssetPath;

	return true;
}

bool Editor::createScene() {
	std::string newSceneFile = CitronIO::IO::saveFileDialog(
		"Scene", CITRON_SCENE_FILE_ENDING, nullptr, 0);
	if (newSceneFile.empty()) {
		CITRON_CLIENT_WARN("Scene: {} was not saved",
						   editorContext.getCurrentScene()->getName());
		return false;
	}
	CitronIO::IO::createFile(newSceneFile);
	return openScene(newSceneFile);
}

bool Editor::openProject(std::string projectFilePath) {
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

void Editor::saveCurrentScene() {
	if (editorContext.currentlyEditedSceneAssetPath.empty() ||
		!CitronIO::IO::fileExists(
			editorContext.currentlyEditedSceneAssetPath)) {
		createScene();
	}

	FileStreamWriter fwriter =
		FileStreamWriter(editorContext.currentlyEditedSceneAssetPath);
	editorContext.getCurrentScene()->serialize(fwriter);

	CITRON_CLIENT_INFO(
		"Scene: {} saved to {}: ", editorContext.getCurrentScene()->getName(),
		editorContext.currentlyEditedSceneAssetPath);
}
