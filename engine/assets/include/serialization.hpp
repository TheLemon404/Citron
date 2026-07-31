#pragma once

#include "citron_exports.hpp"
#include "logger.hpp"
#include <cereal-yaml/archives/yaml.hpp>
#include <cereal/cereal.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace CitronAssets {

class StreamReader;
class StreamWriter;

class CITRON_ASSETS_API ISerializable {
  public:
	virtual void serialize(StreamWriter &writer) = 0;
	virtual void deserialize(StreamReader &reader) = 0;
};

class CITRON_ASSETS_API StreamWriter {
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
	template <typename T>
	void operator()(const T &data) {
		std::stringstream os;
		{
			cereal::YAMLOutputArchive archive(os);
			archive(data);
		}
		writeString(os.str());
	}
};

class CITRON_ASSETS_API FileStreamWriter : public StreamWriter {
  public:
	FileStreamWriter(const std::filesystem::path &filename)
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

class CITRON_ASSETS_API CITRON_ASSETS_API MemoryStreamWriter : public StreamWriter {
  public:
	void writeData(const void *data, size_t size) override;
	void writeString(const std::string &str) override;
};

class CITRON_ASSETS_API NetworkStreamWriter : public StreamWriter {
  public:
	void writeData(const void *data, size_t size) override;
	void writeString(const std::string &str) override;
};

class CITRON_ASSETS_API StreamReader {
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
	template <typename T>
	void operator()(T &data) {
		std::string str;
		readString(str);
		std::stringstream ss(str);
		{
			cereal::YAMLInputArchive archive(ss);
			archive(data);
		}
	}
};

class CITRON_ASSETS_API FileStreamReader : public StreamReader {
  public:
	FileStreamReader(const std::filesystem::path &filename)
		: stream(filename, std::ios::binary) {
		if (!stream.is_open()) {
			CITRON_CORE_ERROR("Failed to open file: {}", filename.string());
		}

		totalFileBytes = stream.seekg(0, std::ios::end).tellg();
		stream.seekg(0, std::ios::beg);
	}

	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;

  private:
	size_t totalFileBytes = 0;
	std::ifstream stream;
};

class CITRON_ASSETS_API MemoryStreamReader : public StreamReader {
  public:
	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;
};

class CITRON_ASSETS_API NetworkStreamReader : public StreamReader {
  public:
	void readData(void *data, size_t size) override;
	void readString(std::string &str) override;
};

} // namespace CitronAssets
