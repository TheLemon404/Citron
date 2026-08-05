//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#pragma once

#include "citron_exports.hpp"
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

namespace CitronCore {
struct CITRON_CORE_API ProfileResult {
	std::string Name;
	long long Start, End;
	uint32_t ThreadID;
};

struct CITRON_CORE_API InstrumentationSession {
	std::string Name;
};

class CITRON_CORE_API Instrumentor {
  private:
	InstrumentationSession *m_CurrentSession;
	std::ofstream m_OutputStream;
	int m_ProfileCount;

  public:
	Instrumentor();
	void BeginSession(const std::string &name, const std::string &filepath = "results.json");
	void EndSession();
	void WriteProfile(const ProfileResult &result);
	void WriteHeader();
	void WriteFooter();
	static Instrumentor &Get();
};

class CITRON_CORE_API InstrumentationTimer {
  public:
	InstrumentationTimer(const char *name)
		: m_Name(name), m_Stopped(false) {
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~InstrumentationTimer() {
		if (!m_Stopped)
			Stop();
	}

	void Stop() {
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Instrumentor::Get().WriteProfile({m_Name, start, end, threadID});

		m_Stopped = true;
	}

  private:
	const char *m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	bool m_Stopped;
};
} // namespace CitronCore

#define CITRON_PROFILE_BEGIN_SESSION(name, filepath) Instrumentor::Get().BeginSession(name, filepath);
#define CITRON_PROFILE_END_SESSION() Instrumentor::Get().EndSession();
#define CITRON_PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name);
#define CITRON_PROFILE_FUNCTION() CITRON_PROFILE_SCOPE(__PRETTY_FUNCTION__)
