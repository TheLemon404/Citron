#pragma once

#if defined(_WIN32)
#if defined(CITRON_GRAPHICS_BUILD_DLL)
#define CITRON_GRAPHICS_API __declspec(dllexport)
#else
#define CITRON_GRAPHICS_API __declspec(dllimport)
#endif
#else
#define CITRON_GRAPHICS_API
#endif
