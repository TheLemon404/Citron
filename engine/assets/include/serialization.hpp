#pragma once

#include "alpaca/detail/options.h"
#include "logger.hpp"

#include <alpaca/alpaca.h>
#include <entt/entt.hpp>
#include <fstream>

namespace CitronAssets {

class StreamReader;
class StreamWriter;

template <typename T> class ISerializable {
  public:
	virtual void serialize(StreamWriter &writer) = 0;
	virtual void deserialize(StreamReader &reader) = 0;
};

class StreamWriter {
  public:
	virtual void writeData(const void *data, size_t size) = 0;
	virtual void writeString(const std::string &str) = 0;

	// Need to impliment these methods for EnTT's snapshot integration
	void operator()(entt::entity entity) {
		writeData(reinterpret_cast<const char *>(&entity),
				  sizeof(entt::entity));
	}
	void operator()(std::underlying_type_t<entt::entity> size) {
		writeData(reinterpret_cast<const char *>(&size), sizeof(size));
	}
	template <typename T> void operator()(const T &data) {
		std::vector<uint8_t> bytes;
		alpaca::serialize<alpaca::options::fixed_length_encoding>(data, bytes);
		uint64_t len = bytes.size();
		writeData(&len, sizeof(len));
		writeData(bytes.data(), len);
	}
};

class FileStreamWriter : public StreamWriter {
  public:
	FileStreamWriter(const std::string &filename)
		: stream(filename, std::ios::binary) {
		stream.clear();
		stream.seekp(0);
	}
	~FileStreamWriter() { stream.close(); }

	void writeData(const void *data, size_t size) override;
	void writeString(const std::string &str) override;

  private:
	std::ofstream stream;
};

class MemoryStreamWriter : public StreamWriter {
  public:
	void writeData(const void *data, size_t size) override;
	void writeString(const std::string &str) override;
};

class NetworkStreamWriter : public StreamWriter {
  public:
	void writeData(const void *data, size_t size) override;
	void writeString(const std::string &str) override;
};

class StreamReader {
  public:
	virtual void readData(void *data, size_t size) = 0;
	virtual void readString(std::string &str) = 0;

	// Need to impliment these methods for EnTT's snapshot integration
	void operator()(entt::entity &entity) {
		readData(&entity, sizeof(entt::entity));
	}
	void operator()(std::underlying_type_t<entt::entity> &size) {
		readData(&size, sizeof(size));
	}
	template <typename T> void operator()(T &data) {
		uint64_t len;
		readData(&len, sizeof(len));
		CITRON_CORE_INFO("About to read component len={}", len); // <-- add this

		std::vector<uint8_t> bytes(len);
		readData(bytes.data(), len);
		std::error_code ec;
		data = alpaca::deserialize<alpaca::options::fixed_length_encoding, T>(
			bytes, ec);
		if (ec) {
			CITRON_CORE_ERROR("Serialization failed with error: {}",
							  ec.message());
		}
	}
};

class FileStreamReader : public StreamReader {
  public:
	FileStreamReader(const std::string &filename)
		: stream(filename, std::ios::binary) {
		if (!stream.is_open()) {
			CITRON_CORE_ERROR("Failed to open file: {}", filename);
		}
	}

	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;

  private:
	std::ifstream stream;
};

class MemoryStreamReader : public StreamReader {
  public:
	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;
};

class NetworkStreamReader : public StreamReader {
  public:
	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;
};

} // namespace CitronAssets
