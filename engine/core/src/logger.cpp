#include "logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

using namespace CitronCore;

std::shared_ptr<spdlog::logger> Logger::coreLogger = nullptr;
std::shared_ptr<spdlog::logger> Logger::clientLogger = nullptr;

void Logger::init() {
	spdlog::set_pattern("%^[%T] [Thread: %t] %n: %v%$");

	coreLogger = spdlog::stdout_color_mt("Citron");
	coreLogger->set_level(spdlog::level::trace);

	clientLogger = spdlog::stdout_color_mt("Citron Client");
	clientLogger->set_level(spdlog::level::trace);
}
