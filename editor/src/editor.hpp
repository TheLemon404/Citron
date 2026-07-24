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

class Editor : public CitronCore::App {
  public:
	Editor(const std::string &projectFilePath);
	inline static Editor &get() { return (Editor &)App::get(); }

	EditorContext &getEditorContext() { return editorContext; }

	void init();
	void close();
	void update();
	void onEvent(CitronCore::Event &e);

	bool openScene(std::string sceneAssetPath);
	bool createScene();

	bool openProject(std::string projectFilePath);

	void saveCurrentScene();

  private:
	EditorContext editorContext;
};
