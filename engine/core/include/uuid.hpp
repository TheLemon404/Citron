#pragma once

#include "citron_exports.hpp"

#include <cstdint>
#include <iostream>

namespace CitronCore {

class CITRON_CORE_API UUID {
  public:
	UUID();
	UUID(uint32_t uuid);
	UUID(const UUID &other) = default;

	static uint32_t nullID;

	bool operator==(const uint32_t uuid) const { return m_uuid == uuid; }
	bool operator==(const int uuid) const { return m_uuid == (uint32_t)uuid; }
	bool operator==(const UUID &other) const { return m_uuid == other.m_uuid; }
	operator uint32_t() const { return m_uuid; }

  private:
	uint32_t m_uuid;
};

} // namespace CitronCore

namespace std {
template <>
struct hash<CitronCore::UUID> {
	std::size_t operator()(const CitronCore::UUID &uuid) const {
		return std::hash<uint32_t>()((uint32_t)uuid);
	}
};
} // namespace std
