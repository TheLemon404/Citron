#pragma once

#include "scripting_exports.hpp"
#include <filesystem>
#include <vector>

extern "C" CITRON_SCRIPTING_API void registerProjectTypes();

#define CITRON_COMPONENT(name)
#define CITRON_FIELD(name)
#define CITRON_SYSTEM(name)

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
	void init(const std::filesystem::path projectRootFolder);
	void buildScripts(const std::filesystem::path projectRootFolder);

  private:
	void generateCompilationFiles(const std::filesystem::path projectRootFolder);
	void parseAndRegisterUserTypes(const std::filesystem::path projectRootFolder);

	std::vector<ScriptingSDKModule> scriptingSKDs;
};
} // namespace CitronScripting
