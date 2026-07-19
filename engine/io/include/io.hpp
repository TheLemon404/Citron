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
	static void overwriteFile(const std::string &path,
							  const std::string &content);
};
} // namespace CitronIO
