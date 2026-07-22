#pragma once

#include <concepts>
#include <memory>
#include <serialization.hpp>
#include <string>

namespace CitronAssets {
enum class AssetCategory { ENGINE, PROJECT };

template <typename T> class ILoadable {
  public:
	virtual void load(const std::string &source) = 0;
};

template <typename T> class ISaveable {
  public:
	virtual void save(const std::string &assetPath) = 0;
};

template <typename T> class ISerializable {
  public:
	virtual std::string serialize(StreamWriter &writer) const = 0;
	virtual void deserialize(StreamReader &reader) = 0;
};

class Assets {
  public:
	template <typename T>
		requires std::derived_from<T, ILoadable<T>>
	static std::shared_ptr<T> load(const std::string &assetPath);

  private:
	static AssetCategory extractCategory(const std::string &assetPath);
};
} // namespace CitronAssets
