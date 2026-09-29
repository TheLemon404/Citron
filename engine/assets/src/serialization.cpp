#include "serialization.hpp"

#include <cstring>
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

void BufferWriter::writeData(const void *data, size_t size) {
	const uint8_t *bytePtr = reinterpret_cast<const uint8_t *>(data);
	buffer.insert(buffer.end(), bytePtr, bytePtr + size);
}

void BufferWriter::writeString(const std::string &str) {
	size_t size = str.size();
	writeData(&size, sizeof(size));
	writeData(str.data(), str.size());
}

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

void BufferReader::readData(void *data, size_t size) {
	if (cursor + size > bufferSize)
		throw std::runtime_error("BufferStreamReader: readData: out of bounds");

	std::memcpy(data, buffer + cursor, size);
	cursor += size;
}

void BufferReader::readString(std::string &str) {
	size_t size;
	readData(&size, sizeof(size));
	std::string result(size, '\0');
	readData(result.data(), size);
	str = std::move(result);
}

void NetworkStreamReader::readData(void *data, size_t size) {}

void NetworkStreamReader::readString(std::string &str) {}
