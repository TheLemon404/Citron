
#pragma once

#if defined(_WIN32)
#if defined(CITRON_CORE_BUILD_DLL)
#define CITRON_CORE_API __declspec(dllexport)
#else
#define CITRON_CORE_API __declspec(dllimport)
#endif
#else
#define CITRON_CORE_API
#endif
