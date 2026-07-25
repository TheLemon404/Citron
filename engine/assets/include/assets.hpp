#pragma once

#include "uuid.hpp"
#include <concepts>
#include <cstdint>
#include <memory>
#include <serialization.hpp>
#include <unordered_map>

using namespace CitronCore;

namespace CitronAssets {

enum class AssetType : std::size_t {
	UNKNOWN = 0,
	SHADER,
	MATERIAL,
	TEXTURE,
	MESH,
};

struct AssetMetadata {
	uint64_t uuid;
	std::filesystem::path assetPath;
	AssetType assetType;
};

class Asset {
  public:
	Asset(const UUID uuid) : uuid(uuid) {}
	virtual void loadFromFile(const std::string &filepath) = 0;

  protected:
	const UUID uuid;
};

template <typename T>
	requires std::derived_from<T, Asset>
struct AssetReference {
	std::string path;
	uint64_t uuid = UUID::nullID;
	AssetType assetType = AssetType::UNKNOWN;
};

template <typename T>
	requires std::derived_from<T, Asset>
class RuntimeAssetLoader;
template <typename T>
	requires std::derived_from<T, Asset>
class EditorAssetLoader;

class AssetManager {
  public:
	virtual void initializeAssetRegistry() = 0;

	const std::unordered_map<UUID, std::weak_ptr<Asset>> &
	getLoadedAssets() const {
		return loadedAssets;
	}

  protected:
	std::unordered_map<UUID, std::weak_ptr<Asset>> loadedAssets;
};

class EditorAssetManager : public AssetManager {
  public:
	EditorAssetManager(const std::filesystem::path &projectRootPath)
		: projectRootPath(projectRootPath) {}

	template <typename T>
		requires std::derived_from<T, Asset>
	std::shared_ptr<T> get(UUID &uuid) {
		if (loadedAssets.find(uuid) != loadedAssets.end()) {
			return std::dynamic_pointer_cast<T>(loadedAssets[uuid].lock());
		}

		std::shared_ptr<T> asset = std::make_shared<T>(uuid);
		asset->loadFromFile(assetMetadataRegistry[uuid]);
		loadedAssets[uuid] = asset;
		return asset;
	}

	virtual void initializeAssetRegistry() override;

	const AssetMetadata &getAssetMetadata(UUID uuid) {
		return assetMetadataRegistry[uuid];
	}

	void refreshAssetRegistry();

	bool isValidAsset(const std::filesystem::path &path);

	AssetMetadata getAssetMetadataByPath(const std::filesystem::path &path) {
		return assetMetadataByPath[path];
	}

  private:
	bool isKnownAssetFileExtension(std::string extension);

	AssetMetadata loadMetadataFromFile(const std::filesystem::path &metaFile);
	void createMetadataForFile(const std::filesystem::path &file, const std::filesystem::path &metaFile);

	std::unordered_map<uint64_t, AssetMetadata> assetMetadataRegistry;
	std::unordered_map<std::filesystem::path, AssetMetadata> assetMetadataByPath;
	const std::filesystem::path projectRootPath;
};

class RuntimeAssetManager : public AssetManager {
  public:
	RuntimeAssetManager() : AssetManager() {}
	virtual void initializeAssetRegistry() override;
};
} // namespace CitronAssets
