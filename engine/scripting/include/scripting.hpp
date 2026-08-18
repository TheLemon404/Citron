#pragma once

#include "citron_exports.hpp"
#include <filesystem>

extern "C" CITRON_SCRIPTING_API void registerProjectTypes();

namespace CitronScripting {
class CITRON_SCRIPTING_API ScriptingEngine {
  public:
	void buildScripts(const std::filesystem::path projectRootFolder);
};
} // namespace CitronScripting
