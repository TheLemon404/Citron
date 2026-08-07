#include "editor.hpp"
#include "SDL3/SDL_keycode.h"
#include "app.hpp"
#include "assets.hpp"
#include "entt/entity/entity.hpp"
#include "event.hpp"
#include "gui.hpp"
#include "imgui.h"
#include "keyboard.hpp"
#include "logger.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "panel.hpp"
#include "yaml-cpp/node/emit.h"
#include <input.hpp>
#include <io.hpp>
#include <memory>
#include <serialization.hpp>
#include <yaml-cpp/yaml.h>
#include <registry.hpp>
#include <imgui_stdlib.h>
#include "gui_elements.hpp"

Editor::Editor(const std::string &projectFilePath)
	: CitronCore::App(
		  false, projectFilePath) {
	editorContext.projectFilePath = projectFilePath;

	AppContext context = App::getContext();

	ECSRegistry::registerPropertyGuiDrawer<int>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::InputInt(fieldNameId.c_str(), (int *)field);
	});
	ECSRegistry::registerPropertyGuiDrawer<UUID>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		std::string uuidString = std::to_string(*(unsigned int *)field);
		ImGui::InputText(fieldNameId.c_str(), &uuidString, ImGuiInputTextFlags_ReadOnly);
	});
	ECSRegistry::registerPropertyGuiDrawer<float>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::InputFloat(fieldNameId.c_str(), (float *)field);
	});
	ECSRegistry::registerPropertyGuiDrawer<std::string>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string *str = (std::string *)field;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::InputText(fieldNameId.c_str(), str);
	});
	ECSRegistry::registerPropertyGuiDrawer<glm::vec2>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::DragFloat2(fieldNameId.c_str(), (float *)field);

		// colors
		ImVec2 inputRectMin = ImGui::GetItemRectMin();
		ImVec2 inputRectMax = ImGui::GetItemRectMax();
		ImVec2 inputRectBottomMin = ImVec2(inputRectMin.x, inputRectMax.y);
		float perEntryOffset = (inputRectMax.x - inputRectMin.x) / 2.0f;
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddLine(inputRectMin, inputRectBottomMin, ImGui::GetColorU32(xColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset, inputRectBottomMin.y), ImGui::GetColorU32(yColor), 1.5f);
	});
	ECSRegistry::registerPropertyGuiDrawer<glm::vec3>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::DragFloat3(fieldNameId.c_str(), (float *)field);

		// colors
		ImVec2 inputRectMin = ImGui::GetItemRectMin();
		ImVec2 inputRectMax = ImGui::GetItemRectMax();
		ImVec2 inputRectBottomMin = ImVec2(inputRectMin.x, inputRectMax.y);
		float perEntryOffset = (inputRectMax.x - inputRectMin.x) / 3.0f;
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddLine(inputRectMin, inputRectBottomMin, ImGui::GetColorU32(xColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset, inputRectBottomMin.y), ImGui::GetColorU32(yColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset * 2.0f, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset * 2.0f, inputRectBottomMin.y), ImGui::GetColorU32(zColor), 1.5f);
	});
	ECSRegistry::registerPropertyGuiDrawer<glm::quat>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(*(glm::quat *)field));
		ImGui::DragFloat3(fieldNameId.c_str(), &eulerRotation[0]);
		*(glm::quat *)field = glm::quat(glm::radians(eulerRotation));

		// colors
		ImVec2 inputRectMin = ImGui::GetItemRectMin();
		ImVec2 inputRectMax = ImGui::GetItemRectMax();
		ImVec2 inputRectBottomMin = ImVec2(inputRectMin.x, inputRectMax.y);
		float perEntryOffset = (inputRectMax.x - inputRectMin.x) / 3.0f;
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddLine(inputRectMin, inputRectBottomMin, ImGui::GetColorU32(xColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset, inputRectBottomMin.y), ImGui::GetColorU32(yColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset * 2.0f, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset * 2.0f, inputRectBottomMin.y), ImGui::GetColorU32(zColor), 1.5f);
	});
	ECSRegistry::registerPropertyGuiDrawer<glm::vec4>([](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
		void *field = (char *)object + member.offset;
		std::string fieldNameId = "##" + member.fieldName;
		ImGui::DragFloat4(fieldNameId.c_str(), (float *)field);

		// colors
		ImVec2 inputRectMin = ImGui::GetItemRectMin();
		ImVec2 inputRectMax = ImGui::GetItemRectMax();
		ImVec2 inputRectBottomMin = ImVec2(inputRectMin.x, inputRectMax.y);
		float perEntryOffset = (inputRectMax.x - inputRectMin.x) / 4.0f;
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddLine(inputRectMin, inputRectBottomMin, ImGui::GetColorU32(xColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset, inputRectBottomMin.y), ImGui::GetColorU32(yColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset * 2.0f, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset * 2.0f, inputRectBottomMin.y), ImGui::GetColorU32(zColor), 1.5f);
		drawList->AddLine(ImVec2(inputRectMin.x + perEntryOffset * 3.0f, inputRectMin.y), ImVec2(inputRectBottomMin.x + perEntryOffset * 3.0f, inputRectBottomMin.y), ImGui::GetColorU32(wColor), 1.5f);
	});
	ECSRegistry::registerPropertyGuiDrawer<AssetReference<Mesh>>(
		[context](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
			void *field = (char *)object + member.offset;
			std::string fieldNameId = "##" + member.fieldName;
			GuiElements::drawAssetReferenceComponentGui<Mesh>(fieldNameId, *(AssetReference<Mesh> *)field, context);
		});
	ECSRegistry::registerPropertyGuiDrawer<AssetReference<Material>>(
		[context](const Member &member, void *object, CitronAssets::AssetManager &assetManager) {
			void *field = (char *)object + member.offset;
			std::string fieldNameId = "##" + member.fieldName;
			GuiElements::drawAssetReferenceComponentGui<Material>(fieldNameId, *(AssetReference<Material> *)field, context);
		});
}

void Editor::init() {
	App::init();

	openProject(editorContext.projectFilePath.string());

	assetManager.initializeAssetRegistry();

	pushLayer<GuiLayer>(getContext());
}

void Editor::update() {
	App::update();
}

void Editor::close() {
	App::close();
	if (!editorContext.projectFilePath.empty()) {
		if (!editorContext.currentlyEditedSceneAssetPath.empty()) {
			YAML::Node projectFileNode =
				YAML::LoadFile(editorContext.projectFilePath.string());
			projectFileNode["last_scene"] =
				editorContext.currentlyEditedSceneAssetPath.string();
			CitronIO::IO::writeFile(editorContext.projectFilePath,
									YAML::Dump(projectFileNode));

			saveCurrentScene();
		}
	}
}

bool Editor::openScene(std::string sceneAssetPath) {
	CITRON_CLIENT_INFO("Opening scene file: " + sceneAssetPath);

	if (!CitronIO::IO::fileExists(sceneAssetPath))
		return false;

	try {
		FileStreamReader reader = FileStreamReader(sceneAssetPath);
		Editor::get().sceneManager.setActiveScene(std::make_shared<Scene>(""));
		Editor::get().sceneManager.getActiveScene()->deserialize(reader);
		editorContext.currentlyEditedSceneAssetPath = sceneAssetPath;
		editorContext.setCurrentlySelectedItem(entt::null);
		return true;
	} catch (std::exception &e) {
		CITRON_CLIENT_ERROR("Failed to open scene file: {}", e.what());
		return false;
	}
}

bool Editor::createScene() {
	std::string newSceneFile = CitronIO::IO::saveFileDialog(
		"Scene", CITRON_SCENE_FILE_ENDING, nullptr, 0);
	if (newSceneFile.empty()) {
		CITRON_CLIENT_WARN("Scene: {} was not saved",
						   Editor::get().sceneManager.getActiveScene()->getName());
		return false;
	}
	CitronIO::IO::createFile(newSceneFile);
	FileStreamWriter writer = FileStreamWriter(newSceneFile);
	Editor::get().sceneManager.setActiveScene(std::make_shared<Scene>(""));
	Editor::get().sceneManager.getActiveScene()->serialize(writer);
	editorContext.currentlyEditedSceneAssetPath = newSceneFile;
	editorContext.setCurrentlySelectedItem(entt::null);
	return true;
}

bool Editor::openProject(std::string projectFilePath) {
	CITRON_CORE_ASSERT(!projectFilePath.empty(), "projectFilePath is empty");
	if (projectFilePath.empty())
		return false;

	editorContext.projectFilePath = projectFilePath;
	editorContext.currentlyEditedSceneAssetPath = "";
	YAML::Node projectFileNode = YAML::LoadFile(projectFilePath);

	if (projectFileNode["last_scene"].IsDefined() &&
		!projectFileNode["last_scene"].IsNull()) {
		std::string lastEditedSceneFile =
			projectFileNode["last_scene"].as<std::string>();

		if (!openScene(lastEditedSceneFile)) {
			CITRON_CLIENT_ERROR("Failed to load last edited scene file: {}",
								lastEditedSceneFile);
			projectFileNode["last_scene"] = "";
			sceneManager.setActiveScene(std::make_shared<Scene>(""));
		}
	} else {
		sceneManager.setActiveScene(std::make_shared<Scene>(""));
	}

	std::string editorTitle = std::string("Citron Editor: ") +
							  editorContext.projectFilePath.filename().string();
	Editor::get().getContext().window.setName(editorTitle);

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
	Editor::get().sceneManager.getActiveScene()->serialize(fwriter);

	CITRON_CLIENT_INFO("Scene saved");
}

glm::ivec2 Editor::getActiveViewSize() {
	return getLayer<GuiLayer>()->viewPanel.getViewportSize();
}
