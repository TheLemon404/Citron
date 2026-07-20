#pragma once

#include <core.hpp>
#include <string>

namespace CitronIO {
class IO {
  public:
	static std::string readFile(const std::string &path);
	static void createFile(const std::string &path);
	static void writeFile(const std::string &path, const std::string &content);
	static bool fileExists(const std::string &path);
	static void cloneFile(const std::string &srcPath,
						  const std::string &dstPath);

	static std::string openFileDialog(const std::string &filtername,
									  const std::string &filters);
	static std::string saveFileDialog(const std::string &fileName,
									  const std::string &filtername,
									  const std::string &filters,
									  const void *bytes, size_t size);
};
} // namespace CitronIO
