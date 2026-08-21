#pragma once

#include "citron_exports.hpp"
#include <filesystem>
#include <vector>

extern "C" CITRON_SCRIPTING_API void registerProjectTypes();

namespace CitronScripting {
struct ScriptingSDKModule {
	const std::string name;
	const std::filesystem::path includePath;
	const std::filesystem::path dllPath;
	const std::filesystem::path libPath;

	ScriptingSDKModule(const std::string sdkName);
};

class CITRON_SCRIPTING_API ScriptingEngine {
  public:
	ScriptingEngine();
	void buildScripts(const std::filesystem::path projectRootFolder);

  private:
	std::vector<ScriptingSDKModule> scriptingSKDs;
};
} // namespace CitronScripting
