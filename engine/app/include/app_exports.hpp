#pragma once

#if defined(_WIN32)
#if defined(CITRON_APP_BUILD_DLL)
#define CITRON_APP_API __declspec(dllexport)
#else
#define CITRON_APP_API __declspec(dllimport)
#endif
#else
#define CITRON_APP_API
#endif
