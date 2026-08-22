#pragma once

#if defined(_WIN32)
#if defined(CITRON_ECS_BUILD_DLL)
#define CITRON_ECS_API __declspec(dllexport)
#else
#define CITRON_ECS_API __declspec(dllimport)
#endif
#else
#define CITRON_ECS_API
#endif
