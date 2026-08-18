#include "scripting.hpp"
#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <libloaderapi.h>
#include <functional>
#include <sstream>
#include <system_error>

#ifdef WIN32
#include <minwindef.h>
#elif __APPLE__
// implement mac library loading
#elif __linux__
// implement linux library loading
#endif

using namespace CitronScripting;

using RegisterProjectFn = long long (*)();

void ScriptingEngine::buildScripts(const std::filesystem::path projectRootFolder) {
	std::ofstream cmakeScript(projectRootFolder / "CMakeLists.txt");
	CITRON_CORE_INFO("Generating/opening cmake script file...");

	if (!cmakeScript.is_open()) {
		CITRON_CORE_ERROR("Failed to open/create cmake build file for scripts");
		return;
	}

	cmakeScript << "cmake_minimum_required(VERSION 3.20)\n";
	cmakeScript << "project(scripts LANGUAGES CXX)\n";
	cmakeScript << "set(CMAKE_CXX_STANDARD 26)\n";
	cmakeScript << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
	cmakeScript << "file(GLOB_RECURSE src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/*.cpp)\n";
	cmakeScript << "add_library(scripts SHARED ${src})\n";
	cmakeScript << "target_sources(citron_scripting PRIVATE ${src})\n";
	cmakeScript << "target_include_directories(citron_scripting PUBLIC ${PROJECT_SOURCE_DIR})\n";
	cmakeScript << "target_link_libraries(scripts PRIVATE citron_scripting)\n";
	cmakeScript.close();

	CITRON_CORE_INFO("Configuring and building cmake script project");

	std::stringstream commandStream;
	commandStream << "cmake -S " << projectRootFolder.string() << " -B " << (projectRootFolder / "build").string();
	int cmakeConfigureResult = std::system(commandStream.str().c_str());
	if (cmakeConfigureResult != 0) {
		CITRON_CORE_ERROR("CMake failed to configure script cmake");
		return;
	}

	commandStream.clear();
	commandStream << "cmake --build " << (projectRootFolder / "build").string();
	int cmakeBuildResult = std::system(commandStream.str().c_str());
	if (cmakeBuildResult != 0) {
		CITRON_CORE_ERROR("CMake failed to build script project");
	}

	// ---load types
}
