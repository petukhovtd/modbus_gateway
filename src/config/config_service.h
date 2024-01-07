#pragma once

#include <config/trace_path.h>

#include <common/logger.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

struct ConfigService {
  ConfigService() = default;
  ConfigService(TracePath &tp, const nlohmann::json::value_t &data);

  Logger::LogLevel logLevel = Logger::LogLevel::Info;
  size_t threads = 1;
};

}// namespace modbus_gateway
