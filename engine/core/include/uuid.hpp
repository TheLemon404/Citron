#pragma once

#include <cstdint>
#include <iostream>

namespace CitronCore {

class UUID {
  public:
	UUID();
	UUID(uint64_t uuid);
	UUID(const UUID &other) = default;

	operator uint64_t() const { return m_uuid; }

  private:
	uint64_t m_uuid;
};

} // namespace CitronCore

namespace std {
template <> struct hash<CitronCore::UUID> {
	std::size_t operator()(const CitronCore::UUID &uuid) const {
		return std::hash<uint64_t>()((uint64_t)uuid);
	}
};
} // namespace std
