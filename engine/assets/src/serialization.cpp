#include "serialization.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace CitronAssets;

void FileStreamWriter::writeData(const void *data, size_t size) {
	stream.write(static_cast<const char *>(data), size);
}

void FileStreamWriter::writeString(const std::string &str) {
	stream.write(str.c_str(), str.size());
}

void MemoryStreamWriter::writeData(const void *data, size_t size) {}

void MemoryStreamWriter::writeString(const std::string &str) {}

void NetworkStreamWriter::writeData(const void *data, size_t size) {}

void NetworkStreamWriter::writeString(const std::string &str) {}

void FileStreamReader::readData(void *data, size_t size) {}

void FileStreamReader::readString(std::string &str) {}

void MemoryStreamReader::readData(void *data, size_t size) {}

void MemoryStreamReader::readString(std::string &str) {}

void NetworkStreamReader::readData(void *data, size_t size) {}

void NetworkStreamReader::readString(std::string &str) {}
