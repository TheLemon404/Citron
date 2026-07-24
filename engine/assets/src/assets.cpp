#include "assets.hpp"

#include <core.hpp>
#include <io.hpp>
#include <memory>

using namespace CitronAssets;
using namespace CitronIO;

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::shared_ptr<T> AssetManager::get(UUID uuid, Args... args) {
	if (loadedAssets.find(uuid) != loadedAssets.end()) {
		return static_cast<std::shared_ptr<T>>(loadedAssets[uuid].lock());
	} else {
		std::weak_ptr<T> newlyLoadedAsset = nullptr;
		if (isRuntime) {
			RuntimeAssetLoader<T> loader(*this);
			newlyLoadedAsset = loader.load(args...);
		} else {
			EditorAssetLoader<T> loader(*this);
			newlyLoadedAsset = loader.load(args...);
		}
		if (newlyLoadedAsset != nullptr) {
			loadedAssets[uuid] = newlyLoadedAsset;
			return static_cast<std::shared_ptr<T>>(newlyLoadedAsset.lock());
		}
		CITRON_CORE_ERROR("Failed to load asset with UUID {}, it is likely "
						  "not registered",
						  uuid);
		return nullptr;
	}
}

void EditorAssetManager::createAssetRegistry() {}

void RuntimeAssetManager::createAssetRegistry() {}

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::weak_ptr<T> EditorAssetLoader<T, Args...>::load(UUID uuid, Args... args) {
	std::shared_ptr<T> asset = std::make_shared<T>(args...);
	asset->loadFromFile(assetManager.getAssetPath(uuid));
	return asset;
}

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::weak_ptr<T> RuntimeAssetLoader<T, Args...>::load(UUID uuid, Args... args) {
	std::shared_ptr<T> asset = std::make_shared<T>(args...);

	return asset;
}
