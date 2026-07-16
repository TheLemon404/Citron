#pragma once

namespace CitronCore {} // namespace CitronCore

#define CITRON_CLIENT_ASSERT(x, ...)                                           \
	{                                                                          \
		if (!(x)) {                                                            \
			CITRON_CLIENT_ERROR("assertion failed: {0}", __VA_ARGS__);         \
			__debugbreak();                                                    \
		}                                                                      \
	}

#define CITRON_CORE_ASSERT(x, ...)                                             \
	{                                                                          \
		if (!(x)) {                                                            \
			CITRON_CORE_ERROR("assertion failed: {0}", __VA_ARGS__);           \
			__debugbreak();                                                    \
		}                                                                      \
	}

#define BIT(x) (1 << x)

#define CITRON_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
