#pragma once

#include <app.hpp>
#include <assets.hpp>
#include <concepts>
#include <ecs.hpp>
#include <event.hpp>
#include <texture.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;
using namespace CitronECS;

class Panel {
  public:
	Panel(const std::string &name) : name(name) {};
	virtual ~Panel() = default;

	virtual void onAttach() {};
	virtual void onDetach() {};
	virtual void onUpdate() {};
	virtual void onDraw() {};
	virtual void onEvent(Event &e) {};

	inline const std::string &getName() const { return name; }

  protected:
	const std::string name;
};

struct AssetCard {
	std::string name;
	std::string path;
	bool isDirectory;
	bool selected;
};

class AssetPanel : public Panel {
  public:
	AssetPanel() : Panel("Assets") {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	std::filesystem::path currentDirectory = "";
	bool pendingRefreshDirectory = false;

  private:
	std::shared_ptr<Texture> folderIconTexture = nullptr;
	wgpu::TextureView folderIconTextureView = nullptr;

	void refreshDirectoryListings();
	std::vector<AssetCard> directoryListings;

	int zoomLevel = 200;
};

class ConsolePanel : public Panel {
  public:
	ConsolePanel() : Panel("Console") {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;
};

class OutlinerPanel : public Panel {
  public:
	OutlinerPanel() : Panel("Outliner") {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

  private:
	void showEntityChildTree(entt::entity entity,
							 std::shared_ptr<Scene> &scenecontext);
	bool pendingCreateEntity = false;
	UUID pendingCreateEntityParent = UUID::nullID;
	UUID pendingDeleteEntity = UUID::nullID;
};

class InspectorPanel : public Panel {
  public:
	InspectorPanel() : Panel("Inspector") {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

  private:
	template <typename T>
		requires std::derived_from<T, AssetBase>
	void drawAssetReferenceComponentGui(const std::string assetName,
										AssetReference<T> &assetReference);
};
