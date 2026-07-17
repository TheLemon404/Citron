#pragma once

#include "layer.hpp"

#include <concepts>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace CitronCore {
class LayerStack {
  public:
	template <typename T>
		requires std::derived_from<T, Layer>
	void pushLayer() {
		layers.push_back(std::make_unique<T>());
		layerMap[typeid(T)] = layers.size() - 1;
	}

	template <typename T>
		requires std::derived_from<T, Layer>
	void popLayer() {
		layers.erase(layers.begin() + layerMap[typeid(T)]);
		layerMap.erase(typeid(T));
	}

	template <typename T>
		requires std::derived_from<T, Layer>
	bool hasLayer() {
		return layerMap.contains(typeid(T));
	}

	template <typename T>
		requires std::derived_from<T, Layer>
	T *getLayer() {
		return static_cast<T *>(layers[layerMap.at(typeid(T))].get());
	}

	std::vector<std::unique_ptr<Layer>>::iterator begin() {
		return layers.begin();
	}
	std::vector<std::unique_ptr<Layer>>::iterator end() { return layers.end(); }

  private:
	class key {
		std::type_index type;
	};
	std::unordered_map<std::type_index, size_t> layerMap;
	std::vector<std::unique_ptr<Layer>> layers;
};
} // namespace CitronCore
