#pragma once

#include <core.hpp>
#include <string>
#include <vector>

namespace CitronIO {

class IO {
  public:
	static std::string readFile(const std::string &path);
	static void createFile(const std::string &path);
	static void createDirectory(const std::string &path);
	static void renameDirectory(const std::string &oldPath,
								const std::string &newPath);
	static void deleteDirectory(const std::string &path);

	static void writeFile(const std::string &path, const std::string &content);
	static bool fileExists(const std::string &path);
	static void cloneFile(const std::string &srcPath,
						  const std::string &dstPath);

	static std::string openFileDialog(const std::string &filtername,
									  const std::string &filters);
	static std::string saveFileDialog(const std::string &filtername,
									  const std::string &filters,
									  const void *bytes, size_t size);

	static std::vector<std::string>
	getDirectoryEntries(const std::string &path);

	static bool isDirectory(const std::string &path);

	static std::string getParentDirectory(const std::string &path);
};
} // namespace CitronIO
