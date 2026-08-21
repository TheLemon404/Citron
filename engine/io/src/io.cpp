#include "io.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <logger.hpp>
#include <nfd.h>
#include <winerror.h>
#include <winscard.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

using namespace CitronIO;

std::string IO::readFile(const std::filesystem::path &path) {
	if (!fileExists(path))
		CITRON_CORE_ERROR("File cannot be read from at (does not exist): {}",
						  path.string());

	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	file.close();
	return content;
}

void IO::createFile(const std::filesystem::path &path) {
	std::ofstream file(path);
	file.clear();
	file << "";
	file.close();
}

void IO::createDirectory(const std::filesystem::path &path) {
	std::filesystem::create_directory(path);
}

void IO::renameDirectory(const std::filesystem::path &oldPath,
						 const std::filesystem::path &newPath) {
	std::filesystem::rename(oldPath, newPath);
}

void IO::deleteDirectory(const std::filesystem::path &path) {
	std::filesystem::remove_all(path);
}

void IO::deleteFile(const std::filesystem::path &path) {
	std::filesystem::remove(path);
}

std::vector<std::filesystem::path>
IO::getFilesInDirectory(const std::filesystem::path &path) {
	std::vector<std::filesystem::path> files;
	for (const auto &entry : std::filesystem::directory_iterator(path)) {
		if (!entry.is_directory())
			files.push_back(entry.path());
	}
	return files;
}

std::vector<std::filesystem::path>
IO::getEntriesInDirectory(const std::filesystem::path &path) {
	std::vector<std::filesystem::path> entries;
	for (const auto &entry : std::filesystem::directory_iterator(path)) {
		entries.push_back(entry.path());
	}
	return entries;
}

std::vector<std::filesystem::path>
IO::getAllFilesInDirectory(const std::filesystem::path &path) {
	std::vector<std::filesystem::path> files;
	for (const auto &entry : std::filesystem::directory_iterator(path)) {
		if (!entry.is_directory())
			files.push_back(entry.path());
		else {
			std::vector<std::filesystem::path> subFiles =
				getAllFilesInDirectory(entry.path());
			files.insert(files.end(), subFiles.begin(), subFiles.end());
		}
	}
	return files;
}

void IO::moveFileOrFolder(const std::filesystem::path &srcPath,
						  const std::filesystem::path &dstPath) {
	std::filesystem::rename(srcPath, dstPath / srcPath.filename());
}

void IO::openFileExplorer(const std::filesystem::path &path) {
	std::system(("explorer " + path.string()).c_str());
}

void IO::writeFile(const std::filesystem::path &path,
				   const std::string &content) {
	if (!fileExists(path))
		CITRON_CORE_ERROR("File cannot be written to at (does not exist): {}",
						  path.string());

	std::ofstream file(path);
	file.clear();
	file << content;
	file.close();
}

bool IO::fileExists(const std::filesystem::path &path) {
	return std::ifstream(path).good();
}

void IO::cloneFile(const std::filesystem::path &srcPath,
				   const std::filesystem::path &dstPath) {
	if (!fileExists(srcPath))
		CITRON_CORE_ERROR("File cannot be cloned from (does not exist): {}",
						  srcPath.string());

	std::ifstream srcFile(srcPath);
	std::ofstream dstFile(dstPath);
	dstFile << srcFile.rdbuf();
	srcFile.close();
	dstFile.close();
}

std::string IO::openFileDialog(const std::string &filtername,
							   const std::string &filters) {
	std::string result = "";

	NFD_Init();

	nfdu8char_t *outPath;
	nfdu8filteritem_t filt[1] = {{filtername.c_str(), filters.c_str()}};
	nfdopendialogu8args_t args = {0};
	args.filterCount = 1;
	args.filterList = filt;
	nfdresult_t openResult = NFD_OpenDialogU8_With(&outPath, &args);
	if (openResult == NFD_OKAY) {
		result = outPath;
		NFD_FreePath(outPath);
	} else if (openResult == NFD_CANCEL) {
		CITRON_CORE_INFO("Open file dialog cancelled");
	} else {
		CITRON_CORE_ERROR("NFD open file dialog failed with error: {}",
						  NFD_GetError());
	}

	NFD_Quit();

	return result;
}

std::string IO::saveFileDialog(const std::string &filtername,
							   const std::string &filters, const void *bytes,
							   size_t size) {
	std::string result = "";

	NFD_Init();
	nfdu8char_t *outPath;
	nfdu8filteritem_t filt[1] = {{filtername.c_str(), filters.c_str()}};

	nfdsavedialogu8args_t args = {0};
	args.filterCount = 1;
	args.filterList = filt;
	nfdresult_t openResult = NFD_SaveDialogU8_With(&outPath, &args);
	if (openResult == NFD_OKAY) {
		result = outPath;
		NFD_FreePath(outPath);
	} else if (openResult == NFD_CANCEL) {
		CITRON_CORE_INFO("Save file dialog cancelled");
	} else {
		CITRON_CORE_ERROR("NFD save file dialog failed with error: {}",
						  NFD_GetError());
	}

	NFD_Quit();

	return result;
}

std::filesystem::path IO::getRunningExecutablePath() {
#if defined(_WIN32)
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer);
#elif defined(__linux__)
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len != -1) {
		buffer[len] = '\0';
		return std::filesystem::path(buffer);
	}
#elif defined(__APPLE__)
	char buffer[1024];
	uint32_t size = sizeof(buffer);
	if (_NSGetExecutablePath(buffer, &size) == 0) {
		return std::filesystem::path(buffer);
	}
#endif
	return ""; // Fallback
}
