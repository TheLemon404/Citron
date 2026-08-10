#include "assets.hpp"
#include "cereal-yaml/archives/yaml.hpp"
#include "logger.hpp"
#include "serialization.hpp"
#include "yaml-cpp/node/emit.h"

#include <concepts>
#include <core.hpp>
#include <cstdint>
#include <io.hpp>
#include <memory>
#include <yaml-cpp/yaml.h>

using namespace CitronAssets;
using namespace CitronIO;

void EditorAssetManager::serialize(StreamWriter &writer) {
	int count = assetMetadataRegistry.size();
	writer.writeData(&count, sizeof(int));
	for (const auto &pair : assetMetadataRegistry) {
		writer.writeData(&pair.first, sizeof(uint32_t));
		writer.writeString(pair.second.assetPath.string());
		writer.writeData(&pair.second.assetType, sizeof(AssetType));
	}
}

void EditorAssetManager::deserialize(StreamReader &reader) {
	try {
		int count;
		reader.readData(&count, sizeof(int));
		assetMetadataRegistry.clear();
		for (int i = 0; i < count; i++) {
			uint32_t uuid;
			reader.readData(&uuid, sizeof(uint32_t));
			std::string path;
			reader.readString(path);
			AssetType assetType = AssetType::UNKNOWN;
			reader.readData(&assetType, sizeof(AssetType));
			AssetMetadata metadata = {};
			metadata.uuid = uuid;
			metadata.assetPath = std::filesystem::path(path);
			metadata.assetType = assetType;
			assetMetadataRegistry[uuid] = metadata;
			filepathToUUID[metadata.assetPath] = uuid;
		}
	} catch (const std::exception &e) {
		CITRON_CORE_ERROR("Failed to deserialize asset registry cache with error: {}", e.what());
		CITRON_CORE_ERROR("Deleting corrupted registry cache...");
		CitronIO::IO::writeFile(projectRootPath / "registry.cache", "");
	}
}

std::unordered_map<UUID, std::shared_ptr<AssetBase>> &AssetManagerBase::getLoadedAssets() {
	return loadedAssets;
}

void AssetManagerBase::registerAssetImporter(AssetType type, std::shared_ptr<AssetImporter> importer) {
	assetImporters[type] = importer;
	for (auto ext : importer->getAssetFileExtensions()) {
		fileExtensionToAssetType[ext] = type;
	}
}

bool AssetManagerBase::isKnownAssetFileExtension(std::string extension) {
	return fileExtensionToAssetType.contains(extension);
}

AssetType AssetManagerBase::getAssetTypeFromExtension(std::string extension) {
	return fileExtensionToAssetType[extension];
}

void EditorAssetManager::initializeAssetRegistry() {
	if (std::filesystem::exists(projectRootPath / "registry.cache")) {
		FileStreamReader fileStreamReader(projectRootPath / "registry.cache");
		deserialize(fileStreamReader);
	}
	refreshAssetRegistry();
}

std::shared_ptr<AssetBase> EditorAssetManager::getAsset(const UUID uuid) {
	if (loadedAssets.contains(uuid)) {
		return loadedAssets[uuid];
	}

	if (!assetMetadataRegistry.contains(uuid)) {
		CITRON_CORE_ERROR("Asset registry does not contain metadata for uuid: {}", (uint32_t)uuid);
		return nullptr;
	}
	AssetMetadata metadata = assetMetadataRegistry[uuid];
	if (!assetImporters.contains(metadata.assetType)) {
		CITRON_CORE_ERROR("No import method exists for asset type: {}", to_string(metadata.assetType));
	}
	std::shared_ptr<AssetBase> newlyLoadedAsset = assetImporters[metadata.assetType]->importAsset(metadata);
	if (newlyLoadedAsset) {
		loadedAssets[uuid] = newlyLoadedAsset;
	}
	return newlyLoadedAsset;
}

void EditorAssetManager::serializeAssets() {
	CITRON_CORE_INFO("Serializing {} loaded assets", loadedAssets.size());
	for (const auto &[uuid, asset] : loadedAssets) {
		std::shared_ptr<ISerializable> serializableAsset = std::dynamic_pointer_cast<ISerializable>(asset);
		if (serializableAsset && assetMetadataRegistry.contains(uuid)) {
			FileStreamWriter writer(assetMetadataRegistry[uuid].assetPath);
			serializableAsset->serialize(writer);
		}
	}
}

void EditorAssetManager::refreshAssetRegistry() {
	std::vector<std::filesystem::path> filesInProject = IO::getAllFilesInDirectory(projectRootPath);
	for (const std::filesystem::path &file : filesInProject) {
		if (isKnownAssetFileExtension(file.extension().string())) {
			if (!filepathToUUID.contains(file) || !assetMetadataRegistry.contains(filepathToUUID[file])) {
				AssetMetadata metadata = {};
				metadata.uuid = (uint32_t)UUID();
				metadata.assetPath = file;
				metadata.assetType = getAssetTypeFromExtension(file.extension().string());
				assetMetadataRegistry[metadata.uuid] = metadata;
				filepathToUUID[file] = metadata.uuid;
			}
		}
	}
	for (const std::pair<uint32_t, AssetMetadata> &metadata : assetMetadataRegistry) {
		if (!std::filesystem::exists(metadata.second.assetPath)) {
			assetMetadataRegistry.erase(metadata.first);
			filepathToUUID.erase(metadata.second.assetPath);
		}
	}
}

bool EditorAssetManager::isValidAsset(const UUID uuid) {
	return assetMetadataRegistry.contains(uuid);
}

AssetType EditorAssetManager::getAssetType(const UUID uuid) {
	return assetMetadataRegistry[uuid].assetType;
}

void EditorAssetManager::moveAsset(const std::filesystem::path &srcPath, const std::filesystem::path &dstPath) {
	assetMetadataRegistry[filepathToUUID[srcPath]].assetPath = dstPath;
	filepathToUUID[dstPath] = filepathToUUID[srcPath];
	filepathToUUID.erase(srcPath);
}

bool EditorAssetManager::isKnownAssetFileExtension(std::string extension) {
	for (auto importer : assetImporters) {
		if (importer.second->getAssetFileExtensions().contains(extension))
			return true;
	}

	return false;
}

AssetType EditorAssetManager::getAssetTypeFromExtension(std::string extension) {
	for (auto importer : assetImporters) {
		if (importer.second->getAssetFileExtensions().contains(extension))
			return importer.first;
	}

	return AssetType::UNKNOWN;
}

void RuntimeAssetManager::serializeAssets() {}
void RuntimeAssetManager::initializeAssetRegistry() {}
void RuntimeAssetManager::refreshAssetRegistry() {}
std::shared_ptr<AssetBase> RuntimeAssetManager::getAsset(const UUID uuid) {}
bool RuntimeAssetManager::isValidAsset(const UUID uuid) {}
AssetType RuntimeAssetManager::getAssetType(const UUID uuid) {}

AssetManager::AssetManager(const bool isRuntime, std::filesystem::path projectRootPath, EventCallbackFn eventCallback) : isRuntime(isRuntime), projectRootPath(projectRootPath), eventCallback(eventCallback) {
	if (isRuntime) {
		m_assetManager = std::make_unique<RuntimeAssetManager>();
	} else {
		m_assetManager = std::make_unique<EditorAssetManager>(projectRootPath);
	}
}

void AssetManager::initializeAssetRegistry() {
	m_assetManager->initializeAssetRegistry();
	CITRON_CORE_INFO("Initialized Asset Registry");
}

AssetManager::~AssetManager() {
	if (!isRuntime) {
		if (!std::filesystem::exists(projectRootPath / "registry.cache")) {
			IO::createFile(projectRootPath / "registry.cache");
		}
		FileStreamWriter fileStreamWriter(projectRootPath / "registry.cache");
		((EditorAssetManager *)(m_assetManager.get()))->serialize(fileStreamWriter);
	}
}
