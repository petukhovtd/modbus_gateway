#include <common/fmt_logger.h>

namespace modbus_gateway {

void Logger::SetLogLevel(Logger::LogLevel logLevel) {
  spdlog::default_logger()->set_level(static_cast<spdlog::level::level_enum>(logLevel));
}

}// namespace modbus_gateway
