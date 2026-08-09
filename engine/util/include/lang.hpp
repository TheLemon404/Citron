#pragma once

#include "citron_exports.hpp"
#include <cstddef>
#include <tuple>
#include <typeinfo>

// Helper to extract Member and Base types from pointer-to-member
template <class Member, class Base>
std::tuple<Member, Base> get_types(Member Base::*);

template <class TheBase = void, class TT>
inline constexpr std::ptrdiff_t offset_of(TT member) {
	using T = decltype(get_types(std::declval<TT>()));
	using Member = std::tuple_element_t<0, T>;
	using Orig = std::tuple_element_t<1, T>;
	using Base = std::conditional_t<std::is_void_v<TheBase>, Orig, TheBase>;

	// Purely calculate displacement relative to a pseudo-address
	// This side-steps calling constructors, destructors, or generating unions entirely.
	return reinterpret_cast<std::ptrdiff_t>(&((static_cast<const Orig *>(nullptr))->*member));
}

template <auto member, class TheBase = void>
inline constexpr std::ptrdiff_t offset_of() {
	return offset_of<TheBase>(member);
}

class CITRON_UTIL_API Hashing {
  public:
	static size_t hash_combine(size_t seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}

	// A simple constexpr FNV-1a 32-bit hash function
	static constexpr uint32_t hash_32_fnv1a(const char *str) {
		uint32_t hash = 0x811c9dc5;
		while (*str) {
			hash ^= static_cast<uint32_t>(*str++);
			hash *= 0x01000193;
		}
		return hash;
	}

	// Helper to safely get a unique 32-bit hash for any type
	template <typename T>
	static uint32_t typeHash() {
		return hash_32_fnv1a(typeid(T).name());
	}
};
