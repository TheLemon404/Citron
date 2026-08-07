#pragma once

#include "IconsFontAwesome5.h"
#include "IconsFontAwesome6.h"
#include "entt/entity/entity.hpp"
#include "shader.hpp"
#include "view.hpp"
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
	Panel(const std::string &name, AppContext appContext) : name(name), appContext(appContext) {};
	virtual ~Panel() = default;

	virtual void onAttach() = 0;
	virtual void onDetach() = 0;
	virtual void onUpdate() = 0;
	virtual void onDraw() = 0;
	virtual void onEvent(Event &e) = 0;

	inline const std::string &getName() const { return name; }

  protected:
	AppContext appContext;
	const std::string name;
};

struct AssetCard {
	std::string name;
	std::filesystem::path path;
	bool isDirectory;
	bool selected;
};

class AssetPropertiesPanel : public Panel {
  public:
	AssetPropertiesPanel(AppContext appContext) : Panel("Asset Properties", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	void setSelectedAsset(const std::filesystem::path &path);

  private:
	void drawShaderProperties(std::shared_ptr<Shader> shader);
	void drawMaterialProperties(std::shared_ptr<Material> material);
	void drawTextureProperties(std::shared_ptr<Texture> texture);
	void drawMeshProperties(std::shared_ptr<Mesh> mesh);
	void drawGenericProperties(AssetMetadata metadata);

	UUID currentlySelectedAsset = UUID::nullID;
	AssetType currentlySelectedAssetType = CitronAssets::AssetType::UNKNOWN;
	std::filesystem::path currentlySelectedAssetPath = "";
};

class AssetPanel : public Panel {
  public:
	AssetPanel(AppContext appContext, AssetPropertiesPanel &assetPropertiesPanel) : Panel("Assets", appContext), assetPropertiesPanel(assetPropertiesPanel) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	std::filesystem::path currentDirectory = "";
	bool pendingRefreshDirectory = false;

  private:
	AssetPropertiesPanel &assetPropertiesPanel;
	std::shared_ptr<Texture> folderIconTexture = nullptr;
	wgpu::TextureView folderIconTextureView = nullptr;

	void refreshDirectoryListings();
	std::vector<AssetCard> directoryListings;

	int zoomLevel = 100;
};

class ConsolePanel : public Panel {
  public:
	ConsolePanel(AppContext appContext) : Panel("Console", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;
};

class OutlinerPanel : public Panel {
  public:
	OutlinerPanel(AppContext appContext) : Panel("Outliner", appContext) {}

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
	std::shared_ptr<System> pendingDeleteSystem = nullptr;
};

class InspectorPanel : public Panel {
  public:
	InspectorPanel(AppContext appContext) : Panel("Inspector", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	template <typename T>
		requires std::derived_from<T, AssetBase>
	static void drawAssetReferenceComponentGui(const std::string assetName,
											   AssetReference<T> &assetReference,
											   AppContext appContext);

	static bool collapsingHeader(const char *label,
								 const char *icon_open = "+",
								 const char *icon_closed = "-");
};

class AssetRegistryPanel : public Panel {
  public:
	AssetRegistryPanel(AppContext appContext) : Panel("Asset Registry", appContext) {}

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;
	virtual void onEvent(Event &e) override;

	bool showWindow = false;
};
