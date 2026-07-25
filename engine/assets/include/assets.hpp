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

class AssetManager;

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
	AssetManager() {}

	virtual void initializeAssetRegistry() = 0;

	const std::unordered_map<UUID, std::weak_ptr<Asset>> &
	getLoadedAssets() const {
		return loadedAssets;
	}

  protected:
	std::unordered_map<UUID, std::weak_ptr<Asset>> loadedAssets;
};

class EditorAssetManager : public AssetManager,
						   ISerializable<EditorAssetManager> {
  public:
	EditorAssetManager(const std::string &projectRootPath)
		: projectRootPath(projectRootPath),
		  registryCacheFilePath(projectRootPath + "registry.citron") {}
	~EditorAssetManager() {
		FileStreamWriter writer(registryCacheFilePath);
		serialize(writer);
	}

	template <typename T>
		requires std::derived_from<T, Asset>
	std::shared_ptr<T> get(UUID &uuid) {
		if (loadedAssets.find(uuid) != loadedAssets.end()) {
			return std::dynamic_pointer_cast<T>(loadedAssets[uuid].lock());
		}

		std::shared_ptr<T> asset = std::make_shared<T>(uuid);
		asset->loadFromFile(assetIdToPathMap[uuid]);
		loadedAssets[uuid] = asset;
		return asset;
	}

	virtual void initializeAssetRegistry() override;

	virtual void serialize(StreamWriter &writer) override;
	virtual void deserialize(StreamReader &reader) override;

	std::filesystem::path &getAssetPath(UUID uuid) {
		return assetIdToPathMap[uuid];
	}
	UUID getAssetId(const std::string &path) { return assetPathToIdMap[path]; }

	void refreshAssetRegistry();

  private:
	std::vector<std::string> getRegistryFileAssets();

	std::unordered_map<uint64_t, std::filesystem::path> assetIdToPathMap;
	std::unordered_map<std::filesystem::path, uint64_t> assetPathToIdMap;
	const std::string registryCacheFilePath;
	const std::string projectRootPath;
};

class RuntimeAssetManager : public AssetManager {
  public:
	RuntimeAssetManager() : AssetManager() {}
	virtual void initializeAssetRegistry() override;
};
} // namespace CitronAssets
