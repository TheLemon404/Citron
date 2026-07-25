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
	SHADER,
	MATERIAL,
	TEXTURE,
	MESH,
};

class AssetManager;

class Asset {
  public:
	Asset(const UUID uuid) : uuid(uuid) {}
	virtual void loadFromFile(const std::string &filepath);

  private:
	const UUID uuid;
	AssetType type;
};

template <typename T>
	requires std::derived_from<T, Asset>
struct AssetReference {
	std::string path;
	uint64_t uuid = UUID::nullID;
	AssetType typeHash = static_cast<AssetType>(typeid(T).hash_code());
};

class AssetManager {
  public:
	AssetManager(bool isRuntime) : isRuntime(isRuntime) {}

	virtual void initializeAssetRegistry() = 0;

	template <typename T, typename... Args>
		requires std::derived_from<T, Asset>
	std::shared_ptr<T> get(AssetReference<T> &assetReference, Args... args);

	const std::unordered_map<UUID, std::weak_ptr<Asset>> &
	getLoadedAssets() const {
		return loadedAssets;
	}

  private:
	bool isRuntime = false;
	std::unordered_map<UUID, std::weak_ptr<Asset>> loadedAssets;
};

class EditorAssetManager : public AssetManager,
						   ISerializable<EditorAssetManager> {
  public:
	EditorAssetManager(const std::string &projectRootPath)
		: AssetManager(false), projectRootPath(projectRootPath),
		  registryCacheFilePath(projectRootPath + "registry.citron") {}
	~EditorAssetManager() {
		FileStreamWriter writer(registryCacheFilePath);
		serialize(writer);
	}

	virtual void initializeAssetRegistry() override;

	virtual void serialize(StreamWriter &writer) override;
	virtual void deserialize(StreamReader &reader) override;

	std::string &getAssetPath(UUID uuid) { return assetIdToPathMap[uuid]; }
	UUID getAssetId(const std::string &path) { return assetPathToIdMap[path]; }

	void refreshAssetRegistry();

  private:
	std::vector<std::string> getRegistryFileAssets();

	std::unordered_map<uint64_t, std::string> assetIdToPathMap;
	std::unordered_map<std::string, uint64_t> assetPathToIdMap;
	const std::string registryCacheFilePath;
	const std::string projectRootPath;
};

class RuntimeAssetManager : public AssetManager {
  public:
	RuntimeAssetManager() : AssetManager(true) {}
	virtual void initializeAssetRegistry() override;
};

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
class IAssetLoader {
  public:
	virtual std::weak_ptr<T> load(AssetReference<T> &reference, Args... args);
};

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
class EditorAssetLoader : public IAssetLoader<T, Args...> {
  public:
	EditorAssetLoader(EditorAssetManager &assetManager)
		: IAssetLoader<T>(), assetManager(assetManager) {}
	std::weak_ptr<T> load(UUID uuid, Args... args) override;

  private:
	EditorAssetManager &assetManager;
};

template <typename T, typename... Args>
	requires std::derived_from<T, Asset>
class RuntimeAssetLoader : public IAssetLoader<T, Args...> {
  public:
	RuntimeAssetLoader(RuntimeAssetManager &assetManager)
		: IAssetLoader<T>(), assetManager(assetManager) {}
	std::weak_ptr<T> load(UUID uuid, Args... args) override;

  private:
	RuntimeAssetManager &assetManager;
};

} // namespace CitronAssets
