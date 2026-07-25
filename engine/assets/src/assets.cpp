#include "assets.hpp"
#include "alpaca/alpaca.h"
#include "logger.hpp"
#include "serialization.hpp"
#include "yaml-cpp/node/emit.h"

#include <core.hpp>
#include <cstdint>
#include <io.hpp>
#include <memory>
#include <yaml-cpp/yaml.h>

using namespace CitronAssets;
using namespace CitronIO;

std::shared_ptr<AssetBase> AssetImporter::importAsset(AssetMetadata metadata) {
}

void EditorAssetManager::initializeAssetRegistry() {
	std::vector<std::filesystem::path> filesInProject = IO::getAllFilesInDirectory(projectRootPath);
	std::set<std::filesystem::path> metaFilesInProject;
	for (const std::filesystem::path &file : filesInProject) {
		if (file.extension() == ".meta") {
			metaFilesInProject.insert(file);
		}
	}
	for (const std::filesystem::path &file : filesInProject) {
		if (file.extension() != ".meta" && isKnownAssetFileExtension(file.extension().string())) {
			if (!metaFilesInProject.contains(file.string() + ".meta")) {
				IO::createFile(file.string() + ".meta");
				createMetadataForFile(file, file.string() + ".meta");
			}

			AssetMetadata metadata = loadMetadataFromFile(file.string() + ".meta");
			assetMetadataByPath[file] = metadata;
			assetMetadataRegistry[metadata.uuid] = metadata;
		}
	}
}

std::shared_ptr<AssetBase> EditorAssetManager::getAsset(const UUID uuid) {
	if (loadedAssets.find(uuid) != loadedAssets.end()) {
		return loadedAssets[uuid].lock();
	}
	AssetMetadata metadata = assetMetadataRegistry[uuid];
	if (!assetImporters.contains(metadata.assetType)) {
		CITRON_CORE_ERROR("No import method exists for asset type: {}", to_string(metadata.assetType));
	}
	std::shared_ptr<AssetBase> newlyLoadedAsset = assetImporters[metadata.assetType]->importAsset(metadata);
	loadedAssets[uuid] = newlyLoadedAsset;
	return newlyLoadedAsset;
}

void EditorAssetManager::refreshAssetRegistry() {
	std::vector<std::filesystem::path> filesInProject = IO::getAllFilesInDirectory(projectRootPath);
	std::set<std::filesystem::path> metaFilesInProject;
	for (const std::filesystem::path &file : filesInProject) {
		if (file.extension() == ".meta") {
			metaFilesInProject.insert(file);
		}
	}
	for (const std::filesystem::path &file : filesInProject) {
		if (file.extension() != ".meta" && isKnownAssetFileExtension(file.extension().string())) {
			if (!metaFilesInProject.contains(file.string() + ".meta")) {
				IO::createFile(file.string() + ".meta");
				createMetadataForFile(file, file.string() + ".meta");
			}

			AssetMetadata metadata = loadMetadataFromFile(file.string() + ".meta");
			assetMetadataByPath[file] = metadata;
			assetMetadataRegistry[metadata.uuid] = metadata;
		}
	}
	for (const std::pair<uint64_t, AssetMetadata> &metadata : assetMetadataRegistry) {
		if (!std::filesystem::exists(metadata.second.assetPath)) {
			assetMetadataRegistry.erase(metadata.first);
			assetMetadataByPath.erase(metadata.second.assetPath);
			IO::deleteFile(metadata.second.assetPath.string() + ".meta");
		}
	}
}

bool EditorAssetManager::isValidAsset(const UUID uuid) {
	return assetMetadataRegistry.contains(uuid);
}

AssetType EditorAssetManager::getAssetType(const UUID uuid) {
	return assetMetadataRegistry[uuid].assetType;
}

bool EditorAssetManager::isKnownAssetFileExtension(std::string extension) {
	for (char &c : extension) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return extension == ".wgsl" || extension == ".mat" || extension == ".png" || extension == ".jpg" || extension == ".gltf";
}

AssetMetadata EditorAssetManager::loadMetadataFromFile(const std::filesystem::path &metaFile) {
	YAML::Node metaNode = YAML::LoadFile(metaFile.string());
	AssetType type = AssetType::UNKNOWN;
	std::string typeStr = metaNode["assetType"].as<std::string>();
	if (typeStr == "SHADER")
		type = AssetType::SHADER;
	else if (typeStr == "MATERIAL")
		type = AssetType::MATERIAL;
	else if (typeStr == "TEXTURE")
		type = AssetType::TEXTURE;
	else if (typeStr == "MESH")
		type = AssetType::MESH;
	return AssetMetadata(metaNode["uuid"].as<uint64_t>(), metaNode["assetPath"].as<std::string>(), type);
}

void EditorAssetManager::createMetadataForFile(const std::filesystem::path &file, const std::filesystem::path &metaFile) {
	YAML::Node metaNode = YAML::LoadFile(metaFile.string());
	metaNode["uuid"] = (uint64_t)UUID();
	metaNode["assetPath"] = file.string();
	metaNode["assetType"] = to_string(getAssetTypeFromExtension(file.extension().string()));
	IO::writeFile(metaFile, YAML::Dump(metaNode));
}

AssetType EditorAssetManager::getAssetTypeFromExtension(std::string extension) {
	for (char &c : extension) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	if (extension == ".wgsl")
		return AssetType::SHADER;
	else if (extension == ".mat")
		return AssetType::MATERIAL;
	else if (extension == ".png" || extension == ".jpg")
		return AssetType::TEXTURE;
	else if (extension == ".gltf")
		return AssetType::MESH;
	return AssetType::UNKNOWN;
}

void RuntimeAssetManager::initializeAssetRegistry() {}
void RuntimeAssetManager::refreshAssetRegistry() {}
std::shared_ptr<AssetBase> RuntimeAssetManager::getAsset(const UUID uuid) {}
bool RuntimeAssetManager::isValidAsset(const UUID uuid) {}
AssetType RuntimeAssetManager::getAssetType(const UUID uuid) {}
