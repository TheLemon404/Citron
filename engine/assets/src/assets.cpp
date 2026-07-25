#include "assets.hpp"
#include "logger.hpp"
#include "serialization.hpp"

#include <core.hpp>
#include <io.hpp>
#include <memory>

using namespace CitronAssets;
using namespace CitronIO;

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
			// TODO: remove all AssetReferences in components, since all UUIDs
			// will now be invalid
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
				CITRON_CORE_INFO("Asset: {} id {}", file, (uint64_t)uuid);
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
		CITRON_CORE_INFO("Deserialize: uuid={} path={}", path, (uint64_t)uuid);
		assetIdToPathMap[uuid] = path;
		assetPathToIdMap[path] = uuid;
	}
}

void RuntimeAssetManager::initializeAssetRegistry() {}
