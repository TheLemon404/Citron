#include "scripting.hpp"
#include "glm/common.hpp"
#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <libloaderapi.h>
#include <functional>
#include <sstream>
#include <system_error>
#include <io.hpp>

#ifdef WIN32
#include <minwindef.h>
#elif __APPLE__
// implement mac library loading
#elif __linux__
// implement linux library loading
#endif

using namespace CitronScripting;

using RegisterProjectFn = long long (*)();

ScriptingSDKModule::ScriptingSDKModule(const std::string sdkName) : name(sdkName),
																	includePath(CitronIO::IO::getRunningExecutablePath().parent_path() / "sdk/include" / sdkName),
																	dllPath(CitronIO::IO::getRunningExecutablePath().parent_path() / ("lib" + sdkName + ".dll")),
																	libPath(CitronIO::IO::getRunningExecutablePath().parent_path() / ("lib" + sdkName + ".dll.a")) {
}

ScriptingEngine::ScriptingEngine() {
	scriptingSKDs.push_back(ScriptingSDKModule("citron_assets"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_core"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_ecs"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_graphics"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_input"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_io"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_scripting"));
	scriptingSKDs.push_back(ScriptingSDKModule("citron_util"));
}

void ScriptingEngine::buildScripts(const std::filesystem::path projectRootFolder) {
	std::ofstream cmakeScript(projectRootFolder / "CMakeLists.txt");
	CITRON_CORE_INFO("Generating/opening cmake script file...");

	if (!cmakeScript.is_open()) {
		CITRON_CORE_ERROR("Failed to open/create cmake build file for scripts");
		return;
	}

	cmakeScript << "cmake_minimum_required(VERSION 3.20)\n";
	cmakeScript << "project(scripts LANGUAGES CXX)\n";
	cmakeScript << "set(CMAKE_CXX_STANDARD 23)\n";
	cmakeScript << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";

	// directly link to sdk paths
	for (ScriptingSDKModule module : scriptingSKDs) {
		cmakeScript << "add_library(Citron::" << module.name << " SHARED IMPORTED GLOBAL)\n";
		cmakeScript << "set_target_properties(Citron::" << module.name << " PROPERTIES\n";
		cmakeScript << "	IMPORTED_LOCATION \"" << module.dllPath.generic_string() << "\"\n";
		cmakeScript << "	IMPORTED_IMPLIB \"" << module.libPath.generic_string() << "\"\n";
		cmakeScript << "	INTERFACE_INCLUDE_DIRECTORIES \"" << module.includePath.generic_string() << "\"\n";
		cmakeScript << ")\n";
	}

	cmakeScript << "file(GLOB_RECURSE src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/*.cpp)\n";
	cmakeScript << "add_library(scripts SHARED ${src})\n";

	cmakeScript << "target_link_libraries(scripts PRIVATE \n";
	for (ScriptingSDKModule module : scriptingSKDs) {
		cmakeScript << "Citron::" + module.name + " ";
	}
	cmakeScript << ")\n";

	cmakeScript
		<< "target_sources(scripts PRIVATE ${src})\n";
	cmakeScript << "target_include_directories(scripts PUBLIC ${PROJECT_SOURCE_DIR})\n";
	cmakeScript.close();

	CITRON_CORE_INFO("Configuring and building cmake script project");

	auto runCommand = [](const std::string &cmd) {
		CITRON_CORE_INFO("Running: {}", cmd);
		return std::system(cmd.c_str());
	};

	std::string configureCmd = "cmake -S " + projectRootFolder.string() + " -B " + (projectRootFolder / "build").string();
	if (runCommand(configureCmd) != 0) {
		CITRON_CORE_ERROR("CMake failed to configure script cmake");
		return;
	}

	std::string buildCmd = "cmake --build " + (projectRootFolder / "build").string();
	if (runCommand(buildCmd) != 0) {
		CITRON_CORE_ERROR("CMake failed to build script project");
	}

	// ---load types
}
