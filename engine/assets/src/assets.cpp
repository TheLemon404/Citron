#include "assets.hpp"
#include "serialization.hpp"

#include <core.hpp>
#include <io.hpp>
#include <memory>

using namespace CitronAssets;
using namespace CitronIO;

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::shared_ptr<T> AssetManager::get(AssetReference<T> &assetReference,
									 Args... args) {
	if (loadedAssets.find(assetReference.uuid) != loadedAssets.end()) {
		return static_cast<std::shared_ptr<T>>(
			loadedAssets[assetReference.uuid].lock());
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
			assetReference.uuid = UUID();
			loadedAssets[assetReference.uuid] = newlyLoadedAsset;
			return static_cast<std::shared_ptr<T>>(newlyLoadedAsset.lock());
		}
		CITRON_CORE_ERROR("Failed to load asset with UUID {}, it is likely "
						  "not registered",
						  assetReference.uuid);
		return nullptr;
	}
}

void EditorAssetManager::initializeAssetRegistry() {
	if (CitronIO::IO::fileExists(registryCacheFilePath)) {
		try {
			FileStreamReader reader(registryCacheFilePath);
			deserialize(reader);
		} catch (const std::exception &e) {
			CITRON_CORE_ERROR("Failed to deserialize asset registry: {}. "
							  "Deleting and recreating registry file...",
							  e.what());
			CitronIO::IO::deleteFile(registryCacheFilePath);
			CitronIO::IO::createFile(registryCacheFilePath);
		}
	} else {
		CitronIO::IO::createFile(registryCacheFilePath);
	}

	refreshAssetRegistry();
}

void EditorAssetManager::refreshAssetRegistry() {
	std::vector<std::string> registryFileAssets = getRegistryFileAssets();
	std::vector<std::string> filesInProject =
		IO::getAllFilesInDirectory(projectRootPath);
	for (std::string registryFileAsset : registryFileAssets) {
		if (std::find(filesInProject.begin(), filesInProject.end(),
					  registryFileAsset) == filesInProject.end()) {
			uint64_t expiredAssetUUID = assetPathToIdMap[registryFileAsset];
			assetIdToPathMap.erase(expiredAssetUUID);
			assetPathToIdMap.erase(registryFileAsset);
		}
	}

	for (std::string file : filesInProject) {
		if (std::find(registryFileAssets.begin(), registryFileAssets.end(),
					  file) == registryFileAssets.end()) {
			if (!assetPathToIdMap.contains(file)) {
				UUID uuid = UUID();
				assetIdToPathMap[uuid] = file;
				assetPathToIdMap[file] = uuid;
			}
		}
	}

	FileStreamWriter writer(registryCacheFilePath);
	serialize(writer);
}

std::vector<std::string> EditorAssetManager::getRegistryFileAssets() {
	std::vector<std::string> assets;
	UUID uuid;
	std::string path;
	FileStreamReader reader(registryCacheFilePath);
	int numEntries;
	reader.readData(&numEntries, sizeof(int));
	for (int i = 0; i < numEntries; i++) {
		reader.readData(&uuid, sizeof(uint64_t));
		reader.readString(path);
		assets.push_back(path);
	}
	return assets;
}

void EditorAssetManager::serialize(StreamWriter &writer) {
	int numEntries = assetIdToPathMap.size();
	writer.writeData(&numEntries, sizeof(int));
	for (const auto &[uuid, path] : assetIdToPathMap) {
		writer.writeData(&uuid, sizeof(uint64_t));
		writer.writeString(path);
	}
}

void EditorAssetManager::deserialize(StreamReader &reader) {
	UUID uuid;
	std::string path;
	int numEntries;
	reader.readData(&numEntries, sizeof(int));
	for (int i = 0; i < numEntries; i++) {
		reader.readData(&uuid, sizeof(uint64_t));
		reader.readString(path);
		assetIdToPathMap[uuid] = path;
		assetPathToIdMap[path] = uuid;
	}
}

void RuntimeAssetManager::initializeAssetRegistry() {}

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::weak_ptr<T> EditorAssetLoader<T, Args...>::load(UUID uuid, Args... args) {
	std::shared_ptr<T> asset = std::make_shared<T>(uuid, args...);
	asset->loadFromFile(assetManager.getAssetPath(uuid));
	return asset;
}

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
std::weak_ptr<T> RuntimeAssetLoader<T, Args...>::load(UUID uuid, Args... args) {
	std::shared_ptr<T> asset = std::make_shared<T>(uuid, args...);
	return asset;
}
