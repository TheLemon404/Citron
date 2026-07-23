#include "app.hpp"
#include "entt/entity/entity.hpp"
#include "event.hpp"
#include <ecs.hpp>
#include <layer.hpp>
#include <logger.hpp>
#include <window.hpp>

using namespace CitronECS;
using namespace CitronCore;

constexpr const char *CITRON_PROGRAM_FOLDER = "C:/Citron";

class EditorContext {
  public:
	std::string currentlyEditedSceneAssetPath = "";

	std::shared_ptr<Scene> &getCurrentScene() { return currentScene; }
	const entt::entity getCurrentSelectedEntity() {
		return currentSelectedEntity;
	}
	void setCurrentSelectedEntity(const entt::entity entity) {
		currentSelectedEntity = entity;
	}

	void setCurrentScene(std::shared_ptr<Scene> scene) { currentScene = scene; }
	std::string projectFilePath = "";
	std::string projectName = "";
	std::string projectRootFolderPath = "";

  private:
	entt::entity currentSelectedEntity = entt::null;
	std::shared_ptr<Scene> currentScene = nullptr;
};

class EditorLayer : public CitronCore::Layer {
  public:
	EditorLayer();

	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onEvent(CitronCore::Event &e) override;

	bool openSceneFile(std::string sceneAssetPath);
	std::string createSceneFile();

	bool createProject();
	bool openProject(std::string projectFilePath);

	EditorContext &getEditorContext() { return editorContext; }

  private:
	EditorContext editorContext;

	void saveCurrentScene();
};

class Editor : public CitronCore::App {
  public:
	Editor() : CitronCore::App() {}

	virtual void onPushClientLayers() override;
};
