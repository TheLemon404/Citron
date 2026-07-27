#pragma once

#if defined(_WIN32)
#if defined(CITRON_IO_BUILD_DLL)
#define CITRON_IO_API __declspec(dllexport)
#else
#define CITRON_IO_API __declspec(dllimport)
#endif
#else
#define CITRON_IO_API
#endif
