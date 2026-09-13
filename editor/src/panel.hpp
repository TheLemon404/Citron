#pragma once

#include "IconsFontAwesome5.h"
#include "IconsFontAwesome6.h"
#include "entt/entity/entity.hpp"
#include "imgui.h"
#include "shader.hpp"
#include "view.hpp"
#include <app.hpp>
#include <assets.hpp>
#include <concepts>
#include <ecs.hpp>
#include <event.hpp>
#include <texture.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;
using namespace CitronECS;

class Panel {
  public:
	Panel(const std::string &name, AppContext appContext) : name(name), appContext(appContext) {};
	virtual ~Panel() = default;

	virtual void onAttach() = 0;
	virtual void onDetach() = 0;
	virtual void onUpdate() = 0;
	virtual void onDraw() = 0;
	virtual void onEvent(Event &e) = 0;

	inline const std::string &getName() const { return name; }

  protected:
	AppContext appContext;
	const std::string name;
};

struct AssetCard {
	std::string name;
	std::filesystem::path path;
	bool isDirectory;
	bool selected;
};
