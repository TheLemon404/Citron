#pragma once

#include "citron_exports.hpp"

#include "event.hpp"
#include "uuid.hpp"
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <serialization.hpp>
#include <set>
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

constexpr std::string_view to_string(AssetType t) {
	switch (t) {
	case AssetType::SHADER:
		return "SHADER";
	case AssetType::MATERIAL:
		return "MATERIAL";
	case AssetType::TEXTURE:
		return "TEXTURE";
	case AssetType::MESH:
		return "MESH";
	default:
		return "UNKNOWN";
	}
}

struct CITRON_ASSETS_API AssetMetadata {
	uint32_t uuid;
	std::filesystem::path assetPath;
	AssetType assetType;
};

class CITRON_ASSETS_API AssetBase {
  public:
	AssetBase(const UUID uuid) : uuid(uuid) {}
	virtual ~AssetBase() = default;

	const UUID getUUID() { return uuid; }

  protected:
	const UUID uuid;
};

template <typename T, AssetType type>
class Asset : public AssetBase {
  public:
	Asset(const UUID uuid) : AssetBase(uuid) {}
	static constexpr AssetType GetType() { return type; }
};

template <typename T>
	requires std::derived_from<T, AssetBase>
struct AssetReference {
	std::string path;
	uint32_t uuid = UUID::nullID;
	static constexpr AssetType assetType = T::GetType();

	template <class Archive>
	void serialize(Archive &archive) {
		AssetType t = assetType;
		archive(path, uuid, t);
	}
};

class CITRON_ASSETS_API AssetImporter {
  public:
	AssetImporter(std::set<std::string> assetFileExtensions) : assetFileExtensions(assetFileExtensions) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) = 0;

	const std::set<std::string> &getAssetFileExtensions() const {
		return assetFileExtensions;
	}

  protected:
	std::set<std::string> assetFileExtensions;
};

class CITRON_ASSETS_API AssetManagerBase {
  public:
	virtual void initializeAssetRegistry() = 0;
	virtual void refreshAssetRegistry() = 0;

	virtual std::shared_ptr<AssetBase> getAsset(const UUID uuid) = 0;
	virtual bool isValidAsset(const UUID uuid) = 0;
	virtual AssetType getAssetType(const UUID uuid) = 0;

	const std::unordered_map<UUID, std::shared_ptr<AssetBase>> &
	getLoadedAssets();

	void registerAssetImporter(AssetType type, std::shared_ptr<AssetImporter> importer);

	bool isKnownAssetFileExtension(std::string extension);
	AssetType getAssetTypeFromExtension(std::string extension);

	virtual void serializeAssets() = 0;

	std::map<uint32_t, AssetMetadata> &getAssetMetadataRegistry() {
		return assetMetadataRegistry;
	}

  protected:
	std::map<uint32_t, AssetMetadata> assetMetadataRegistry;

	std::unordered_map<std::string, AssetType> fileExtensionToAssetType;
	std::unordered_map<AssetType, std::shared_ptr<AssetImporter>> assetImporters;
	std::unordered_map<UUID, std::shared_ptr<AssetBase>> loadedAssets;
};

class CITRON_ASSETS_API EditorAssetManager : public AssetManagerBase, public ISerializable {
  public:
	EditorAssetManager(const std::filesystem::path &projectRootPath)
		: projectRootPath(projectRootPath) {}

	virtual void initializeAssetRegistry() override;
	virtual std::shared_ptr<AssetBase> getAsset(const UUID uuid) override;

	virtual void serialize(StreamWriter &writer) override;
	virtual void deserialize(StreamReader &reader) override;

	const AssetMetadata &getAssetMetadata(UUID uuid) {
		return assetMetadataRegistry[uuid];
	}

	virtual void serializeAssets() override;

	virtual void refreshAssetRegistry() override;

	virtual bool isValidAsset(const UUID uuid) override;
	virtual AssetType getAssetType(const UUID uuid) override;

	void moveAsset(const std::filesystem::path &srcPath, const std::filesystem::path &dstPath);

	AssetMetadata &getAssetMetadataByPath(const std::filesystem::path &path) {
		return assetMetadataRegistry[filepathToUUID[path]];
	}

	AssetMetadata &getAssetMetadataByUUID(const UUID uuid) {
		return assetMetadataRegistry[uuid];
	}

  private:
	bool isKnownAssetFileExtension(std::string extension);

	AssetType getAssetTypeFromExtension(std::string extension);

	std::unordered_map<std::filesystem::path, uint32_t> filepathToUUID;
	const std::filesystem::path projectRootPath;
};

class CITRON_ASSETS_API RuntimeAssetManager : public AssetManagerBase {
  public:
	virtual void serializeAssets() override;
	virtual void initializeAssetRegistry() override;
	virtual void refreshAssetRegistry() override;
	virtual std::shared_ptr<AssetBase> getAsset(const UUID uuid) override;
	virtual bool isValidAsset(const UUID uuid) override;
	virtual AssetType getAssetType(const UUID uuid) override;
};

class CITRON_ASSETS_API AssetRegistryRefreshEvent : public Event {
  public:
	AssetRegistryRefreshEvent() : Event(nullptr) {}
	std::string toString() const override { return "AppRegistryRefreshEvent"; }
	EVENT_CLASS_TYPE(AssetRegistryRefresh)
	EVENT_CLASS_CATEGORY(EventCategoryApp)
};

using EventCallbackFn = std::function<void(Event &)>;

class CITRON_ASSETS_API AssetManager {
  public:
	AssetManager(const bool isRuntime, std::filesystem::path projectRootPath, EventCallbackFn eventCallback);
	~AssetManager();

	void initializeAssetRegistry();

	void serializeAssets() {
		m_assetManager->serializeAssets();
	}

	void refreshAssetRegistry() {
		AssetRegistryRefreshEvent refreshEvent = AssetRegistryRefreshEvent();
		eventCallback(refreshEvent);
		m_assetManager->refreshAssetRegistry();
	}

	bool isValidAsset(const UUID uuid) {
		return m_assetManager->isValidAsset(uuid);
	}

	AssetType getAssetType(const UUID uuid) {
		return m_assetManager->getAssetType(uuid);
	}

	AssetMetadata &getAssetMetadata(const std::filesystem::path path) {
		if (isRuntime) {
			throw std::runtime_error("getAssetMetadata is not supported in runtime mode");
		}
		return ((EditorAssetManager *)(m_assetManager.get()))->getAssetMetadataByPath(path);
	}

	AssetMetadata &getAssetMetadata(const UUID uuid) {
		if (isRuntime) {
			throw std::runtime_error("getAssetMetadataByUUID is not supported in runtime mode");
		}
		return ((EditorAssetManager *)(m_assetManager.get()))->getAssetMetadataByUUID(uuid);
	}

	void registerAssetImporter(AssetType type, std::shared_ptr<AssetImporter> importer) {
		m_assetManager->registerAssetImporter(type, importer);
	}

	bool isKnownAssetFileExtension(std::string extension) {
		return m_assetManager->isKnownAssetFileExtension(extension);
	}

	AssetType getAssetTypeFromExtension(std::string extension) {
		return m_assetManager->getAssetTypeFromExtension(extension);
	}

	void moveAsset(const std::filesystem::path &srcPath, const std::filesystem::path &dstPath) {
		if (isRuntime) {
			throw std::runtime_error("moveAsset is not supported in runtime mode");
		}
		((EditorAssetManager *)m_assetManager.get())->moveAsset(srcPath, dstPath / srcPath.filename());
	}

	template <typename T>
		requires std::derived_from<T, AssetBase>
	std::shared_ptr<T> getAsset(const UUID uuid) {
		return std::dynamic_pointer_cast<T>(m_assetManager->getAsset(uuid));
	}

	std::map<uint32_t, AssetMetadata> &getAssetMetadataRegistry() {
		return m_assetManager->getAssetMetadataRegistry();
	}

  protected:
	EventCallbackFn eventCallback = nullptr;

  private:
	const std::filesystem::path projectRootPath;
	const bool isRuntime;
	std::unique_ptr<AssetManagerBase> m_assetManager;
};
} // namespace CitronAssets
