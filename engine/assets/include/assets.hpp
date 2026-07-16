#pragma once

#include <concepts>
#include <memory>
#include <string>

namespace CitronAssets {
enum class AssetCategory { ENGINE, PROJECT };

template <typename T> class ILoadable {
  public:
	virtual void load(const std::string &assetSource) = 0;
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
