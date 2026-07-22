#pragma once

#include <cstdint>
#include <iostream>

namespace CitronCore {

class UUID {
  public:
	UUID();
	UUID(uint64_t uuid);
	UUID(const UUID &other) = default;

	bool operator==(const uint64_t uuid) const { return m_uuid == uuid; }
	bool operator==(const int uuid) const { return m_uuid == (uint64_t)uuid; }
	bool operator==(const unsigned int uuid) const {
		return m_uuid == (uint64_t)uuid;
	}
	bool operator==(const UUID &other) const { return m_uuid == other.m_uuid; }
	operator uint64_t() const { return m_uuid; }

  private:
	uint64_t m_uuid;

  public:
	inline static uint64_t nullID = 0;
};

} // namespace CitronCore

namespace std {
template <> struct hash<CitronCore::UUID> {
	std::size_t operator()(const CitronCore::UUID &uuid) const {
		return std::hash<uint64_t>()((uint64_t)uuid);
	}
};
} // namespace std
