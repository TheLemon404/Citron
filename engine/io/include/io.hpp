#pragma once

#include "io_exports.hpp"
#include <core.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace CitronIO {

class CITRON_IO_API IO {
  public:
	static std::string readFile(const std::filesystem::path &path);
	static void createFile(const std::filesystem::path &path);
	static void createDirectory(const std::filesystem::path &path);
	static void renameDirectory(const std::filesystem::path &oldPath,
								const std::filesystem::path &newPath);
	static void deleteDirectory(const std::filesystem::path &path);
	static void deleteFile(const std::filesystem::path &path);

	// not recursive
	static std::vector<std::filesystem::path>
	getFilesInDirectory(const std::filesystem::path &path);

	static std::vector<std::filesystem::path>
	getEntriesInDirectory(const std::filesystem::path &path);

	// recursive
	static std::vector<std::filesystem::path>
	getAllFilesInDirectory(const std::filesystem::path &path);

	static void moveFileOrFolder(const std::filesystem::path &srcPath,
								 const std::filesystem::path &dstPath);

	static void openFileExplorer(const std::filesystem::path &path);

	static void writeFile(const std::filesystem::path &path,
						  const std::string &content);
	static bool fileExists(const std::filesystem::path &path);
	static void cloneFile(const std::filesystem::path &srcPath,
						  const std::filesystem::path &dstPath);

	static std::string openFileDialog(const std::string &filtername,
									  const std::string &filters);
	static std::string saveFileDialog(const std::string &filtername,
									  const std::string &filters,
									  const void *bytes, size_t size);

	static std::filesystem::path getRunningExecutablePath();
};
} // namespace CitronIO
