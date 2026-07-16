#include "io.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace CitronIO;

std::string IO::readFile(const std::string &path) {
	if (!fileExists(path))
		throw std::invalid_argument(
			"File cannot be read from at (does not exist): " + path);

	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	return content;
}

void IO::createFile(const std::string &path) {
	std::ofstream file(path);
	file.clear();
	file << "";
}

void IO::writeFile(const std::string &path, const std::string &content) {
	if (!fileExists(path))
		throw std::invalid_argument(
			"File cannot be written to at (does not exist): " + path);

	std::ofstream file(path);
	file.clear();
	file << content;
}

bool IO::fileExists(const std::string &path) {
	return std::ifstream(path).good();
}

void IO::cloneFile(const std::string &srcPath, const std::string &dstPath) {
	if (!fileExists(srcPath))
		throw std::invalid_argument(
			"File cannot be cloned from at (does not exist): " + srcPath);

	std::ifstream srcFile(srcPath);
	std::ofstream dstFile(dstPath);
	dstFile << srcFile.rdbuf();
}

void IO::overwriteFile(const std::string &path, const std::string &content) {}
