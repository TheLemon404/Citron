#pragma once

#include "uuid.hpp"
#include <concepts>
#include <cstdint>
#include <memory>
#include <serialization.hpp>
#include <unordered_map>

using namespace CitronCore;

namespace CitronAssets {

enum class AssetType {
	SHADER,
	MATERIAL,
	TEXTURE,
	MESH,
};

class Asset {
  public:
	virtual void loadFromFile(const std::string &filepath);

  private:
	const UUID uuid;
	AssetType type;
};

template <typename T>
	requires std::derived_from<T, Asset>
struct AssetReference {
	uint64_t uuid = UUID::nullID;
	std::size_t typeHash = typeid(T).hash_code();
};

class AssetManager {
  public:
	AssetManager(bool isRuntime) : isRuntime(isRuntime) {}

	virtual void createAssetRegistry() = 0;

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

class EditorAssetManager : public AssetManager {
  public:
	EditorAssetManager(const std::string &projectRootPath)
		: AssetManager(false), projectRootPath(projectRootPath) {}

	virtual void createAssetRegistry() override;

	std::string &getAssetPath(UUID uuid) { return assetPathMap[uuid]; }

  private:
	std::unordered_map<UUID, std::string> assetPathMap;
	const std::string projectRootPath;
};

class RuntimeAssetManager : public AssetManager {
  public:
	RuntimeAssetManager() : AssetManager(true) {}
	virtual void createAssetRegistry() override;
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
