#include "serialization.hpp"

#include <fstream>
#include <string>

using namespace CitronAssets;

void FileStreamWriter::writeData(const void *data, size_t size) {
	stream.write(static_cast<const char *>(data), size);
}

void FileStreamWriter::writeString(const std::string &str) {
	size_t size = str.size();
	writeData(&size, sizeof(size));
	writeData(str.data(), str.size());
}

void MemoryStreamWriter::writeData(const void *data, size_t size) {}

void MemoryStreamWriter::writeString(const std::string &str) {}

void NetworkStreamWriter::writeData(const void *data, size_t size) {}

void NetworkStreamWriter::writeString(const std::string &str) {}

void FileStreamReader::readData(void *data, size_t size) {
	stream.read(static_cast<char *>(data), size);
}

void FileStreamReader::readString(std::string &str) {
	size_t size;
	readData(&size, sizeof(size));
	std::string result(size, '\0');
	readData(result.data(), size);
	str = std::move(result);
}

void MemoryStreamReader::readData(void *data, size_t size) {}

void MemoryStreamReader::readString(std::string &str) {}

void NetworkStreamReader::readData(void *data, size_t size) {}

void NetworkStreamReader::readString(std::string &str) {}
