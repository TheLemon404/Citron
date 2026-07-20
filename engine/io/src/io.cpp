#include "io.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <logger.hpp>
#include <nfd.h>
#include <stdexcept>
#include <winerror.h>
#include <winscard.h>

using namespace CitronIO;

std::string IO::readFile(const std::string &path) {
	if (!fileExists(path))
		CITRON_CORE_ERROR("File cannot be read from at (does not exist): {}",
						  path);

	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	file.close();
	return content;
}

void IO::createFile(const std::string &path) {
	std::ofstream file(path);
	file.clear();
	file << "";
	file.close();
}

void IO::writeFile(const std::string &path, const std::string &content) {
	if (!fileExists(path))
		CITRON_CORE_ERROR("File cannot be written to at (does not exist): {}",
						  path);

	std::ofstream file(path);
	file.clear();
	file << content;
	file.close();
}

bool IO::fileExists(const std::string &path) {
	return std::ifstream(path).good();
}

void IO::cloneFile(const std::string &srcPath, const std::string &dstPath) {
	if (!fileExists(srcPath))
		CITRON_CORE_ERROR("File cannot be cloned from (does not exist): {}",
						  srcPath);

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
