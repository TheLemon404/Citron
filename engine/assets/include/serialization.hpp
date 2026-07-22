#pragma once

#include <fstream>

namespace CitronAssets {
class StreamWriter {
  public:
	virtual void writeData(const void *data, size_t size) = 0;
	virtual void writeString(const std::string &str) = 0;
};

class FileStreamWriter : public StreamWriter {
  public:
	FileStreamWriter(const std::string &filename) : stream(filename) {}

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
};

class FileStreamReader : public StreamReader {
  public:
	FileStreamReader(const std::string &filename) : stream(filename) {}

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
