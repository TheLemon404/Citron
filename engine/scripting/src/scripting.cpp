#include "scripting.hpp"
#include "glm/common.hpp"
#include "logger.hpp"
#include "registry.hpp"
#include <errhandlingapi.h>
#include <filesystem>
#include <fstream>
#include <libloaderapi.h>
#include <functional>
#include <sstream>
#include <system_error>
#include <io.hpp>
#include <winnt.h>

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

void ScriptingEngine::init(const std::filesystem::path projectRootFolder) {
	if (CitronIO::IO::fileExists(projectRootFolder / "build" / "libscripts.dll")) {
		parseAndRegisterUserTypes(projectRootFolder);
	}
}

void ScriptingEngine::buildScripts(const std::filesystem::path projectRootFolder) {
	generateCompilationFiles(projectRootFolder);
	parseAndRegisterUserTypes(projectRootFolder);
}

void ScriptingEngine::parseAndRegisterUserTypes(const std::filesystem::path projectRootFolder) {
	bool containsSingleRegisterFunction = false;
	for (const auto &file : CitronIO::IO::getAllFilesInDirectory(projectRootFolder)) {
		if (file.extension() == ".cpp") {
			std::ifstream scriptSource(file);
			if (!scriptSource.is_open()) {
				CITRON_CORE_ERROR("Failed to open script source file: {}", file.string());
				continue;
			}
			std::string line;
			while (std::getline(scriptSource, line)) {
				if (line.contains("void registerScriptTypes()")) {
					if (!containsSingleRegisterFunction) {
						containsSingleRegisterFunction = true;
					} else {
						CITRON_CORE_WARN("Found multiple registerScriptTypes() functions in script: {}", file.string());
						return;
					}
				}
			}
		}
	}
	if (!containsSingleRegisterFunction) {
		CITRON_CORE_WARN("No registerScriptTypes() function found in scripts (add 'extern 'C' __declspec(dllexport) void registerScriptTypes()' somewhere in your project and register your types)");
		return;
	}

	CITRON_CORE_INFO("Registering user types...");

#if defined(__WIN32)
	if (HMODULE scriptModulePreviousHandle = GetModuleHandle("libscripts.dll")) {
		CITRON_CORE_INFO("Unloading previous libscripts.dll...");
		FreeLibrary(scriptModulePreviousHandle);
	}
	HMODULE scriptModule = LoadLibrary((projectRootFolder / "build" / "libscripts.dll").string().c_str());
	if (scriptModule == NULL) {
		CITRON_CORE_ERROR("Failed to load scripts.dll: {}", GetLastError());
		return;
	}

	long long (*registerScriptTypes)() = GetProcAddress(scriptModule, "registerScriptTypes");
	if (registerScriptTypes == NULL) {
		CITRON_CORE_ERROR("Failed to get registerScriptTypes function: {}", GetLastError());
		return;
	}
	registerScriptTypes();

	CITRON_CORE_INFO("Types successfully registered");
#endif
}

void ScriptingEngine::generateCompilationFiles(const std::filesystem::path projectRootFolder) {
	if (HMODULE scriptModulePreviousHandle = GetModuleHandle("libscripts.dll")) {
		CITRON_CORE_INFO("Unloading previous libscripts.dll...");
		FreeLibrary(scriptModulePreviousHandle);
	}

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
	cmakeScript << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";

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

	std::ofstream clangdFile(projectRootFolder / ".clangd");
	clangdFile << "CompileFlags:\n";
	clangdFile << "  Compiler: C:/msys64/clang64/bin/clang++\n";
	clangdFile << "  CompilationDatabase: 'build'\n";
	clangdFile.close();

	CITRON_CORE_INFO("Configuring and building cmake script project");

	auto runCommand = [](const std::string &cmd) {
		CITRON_CORE_INFO("Running: {}", cmd);
		return std::system(cmd.c_str());
	};

	auto quote = [](const std::filesystem::path &path) {
		return "\"" + path.generic_string() + "\"";
	};

	std::filesystem::path buildDir = projectRootFolder / "build";
	std::filesystem::path toolchainBin = "C:/msys64/ucrt64/bin";
	std::filesystem::path ninjaToolchainBin = "C:/msys64/mingw64/bin";

	std::string configureCmd =
		"cmake"
		" -S " +
		quote(projectRootFolder) +
		" -B " + quote(buildDir) +
		" -G Ninja"
		" -DCMAKE_MAKE_PROGRAM:FILEPATH=" +
		quote(ninjaToolchainBin / "ninja.exe") +
		" -DCMAKE_C_COMPILER:FILEPATH=" + quote(ninjaToolchainBin / "gcc.exe") +
		" -DCMAKE_CXX_COMPILER:FILEPATH=" + quote(toolchainBin / "g++.exe");

	if (runCommand(configureCmd) != 0) {
		CITRON_CORE_ERROR("CMake failed to configure script cmake");
		return;
	}

	std::string buildCmd = "cmake --build " + quote(projectRootFolder / "build");
	if (runCommand(buildCmd) != 0) {
		CITRON_CORE_ERROR("CMake failed to build script project");
	}

	CITRON_CORE_INFO("Scripts built successfully");
}
