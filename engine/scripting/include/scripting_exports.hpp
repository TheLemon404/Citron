#pragma once

#if defined(_WIN32)
#if defined(CITRON_SCRIPTING_BUILD_DLL)
#define CITRON_SCRIPTING_API __declspec(dllexport)
#else
#define CITRON_SCRIPTING_API __declspec(dllimport)
#endif
#else
#define CITRON_SCRIPTING_API
#endif
