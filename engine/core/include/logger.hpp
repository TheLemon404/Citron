#pragma once

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace CitronCore {

class Logger {
public:
  static void init();

  inline static std::shared_ptr<spdlog::logger> &getCoreLogger() {
    return coreLogger;
  }
  inline static std::shared_ptr<spdlog::logger> &getClientLogger() {
    return clientLogger;
  }

private:
  static std::shared_ptr<spdlog::logger> coreLogger;
  static std::shared_ptr<spdlog::logger> clientLogger;
};

} // namespace CitronCore

// core macros
#define CITRON_CORE_CRITICAL(...)                                              \
  ::CitronCore::Logger::getCoreLogger()->critical(__VA_ARGS__)
#define CITRON_CORE_ERROR(...)                                                 \
  ::CitronCore::Logger::getCoreLogger()->error(__VA_ARGS__)
#define CITRON_CORE_WARN(...)                                                  \
  ::CitronCore::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define CITRON_CORE_INFO(...)                                                  \
  ::CitronCore::Logger::getCoreLogger()->info(__VA_ARGS__)
#define CITRON_CORE_DEBUG(...)                                                 \
  ::CitronCore::Logger::getCoreLogger()->debug(__VA_ARGS__)
#define CITRON_CORE_TRACE(...)                                                 \
  ::CitronCore::Logger::getCoreLogger()->trace(__VA_ARGS__)

// client macros
#define CITRON_CLIENT_CRITICAL(...)                                            \
  ::CitronCore::Logger::getClientLogger()->critical(__VA_ARGS__)
#define CITRON_CLIENT_ERROR(...)                                               \
  ::CitronCore::Logger::getClientLogger()->error(__VA_ARGS__)
#define CITRON_CLIENT_WARN(...)                                                \
  ::CitronCore::Logger::getClientLogger()->warn(__VA_ARGS__)
#define CITRON_CLIENT_INFO(...)                                                \
  ::CitronCore::Logger::getClientLogger()->info(__VA_ARGS__)
#define CITRON_CLIENT_DEBUG(...)                                               \
  ::CitronCore::Logger::getClientLogger()->debug(__VA_ARGS__)
#define CITRON_CLIENT_TRACE(...)                                               \
  ::CitronCore::Logger::getClientLogger()->trace(__VA_ARGS__)
