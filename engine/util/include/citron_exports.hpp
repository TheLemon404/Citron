#pragma once

#if defined(_WIN32)
#if defined(CITRON_UTIL_BUILD_DLL)
#define CITRON_UTIL_API __declspec(dllexport)
#else
#define CITRON_UTIL_API __declspec(dllimport)
#endif
#else
#define CITRON_UTIL_API
#endif
