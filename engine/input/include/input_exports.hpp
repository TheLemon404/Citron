#pragma once

#if defined(_WIN32)
#if defined(CITRON_INPUT_BUILD_DLL)
#define CITRON_INPUT_API __declspec(dllexport)
#else
#define CITRON_INPUT_API __declspec(dllimport)
#endif
#else
#define CITRON_INPUT_API
#endif
