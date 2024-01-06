#pragma once

#include <spdlog/spdlog.h>

#define MG_TRACE(format, ...) spdlog::trace(format, ##__VA_ARGS__);
#define MG_DEBUG(format, ...) spdlog::debug(format, ##__VA_ARGS__);
#define MG_INFO(format, ...) spdlog::info(format, ##__VA_ARGS__);
#define MG_WARN(format, ...) spdlog::warn(format, ##__VA_ARGS__);
#define MG_ERROR(format, ...) spdlog::error(format, ##__VA_ARGS__);
#define MG_CRIT(format, ...) spdlog::critical(format, ##__VA_ARGS__);

namespace modbus_gateway {

class Logger {
public:
  enum class LogLevel {
    Trace = spdlog::level::trace,
    Debug = spdlog::level::debug,
    Info = spdlog::level::info,
    Warning = spdlog::level::warn,
    Error = spdlog::level::err,
    Critical = spdlog::level::critical,
  };

  static void SetLogLevel(LogLevel logLevel);
};

}// namespace modbus_gateway
