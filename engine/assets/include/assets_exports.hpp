
#pragma once

#if defined(_WIN32)
#if defined(CITRON_ASSETS_BUILD_DLL)
#define CITRON_ASSETS_API __declspec(dllexport)
#else
#define CITRON_ASSETS_API __declspec(dllimport)
#endif
#else
#define CITRON_ASSETS_API
#endif
