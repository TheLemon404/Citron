#include "assets.hpp"

#include <core.hpp>
#include <io.hpp>
#include <memory>
#include <stdexcept>

using namespace CitronAssets;
using namespace CitronIO;

template <typename T>
	requires std::derived_from<T, ILoadable<T>>
std::shared_ptr<T> Assets::load(const std::string &assetPath) {

	AssetCategory category = extractCategory(assetPath);

	std::shared_ptr<T> result = std::make_shared<T>();

	switch (category) {
	case AssetCategory::ENGINE: {
		result->load(IO::readFile(assetPath));
		break;
	}
	case AssetCategory::PROJECT: {
		result->load(IO::readFile(assetPath));
		break;
	}

		return result;
	}
}

AssetCategory Assets::extractCategory(const std::string &assetPath) {
	std::string startingText = assetPath.substr(0, assetPath.find("://"));
	if (startingText == "engine") {
		return AssetCategory::ENGINE;
	} else if (startingText == "assets") {
		return AssetCategory::PROJECT;
	}

	throw std::invalid_argument("Unknown asset category loaded from: " +
								assetPath);
}
