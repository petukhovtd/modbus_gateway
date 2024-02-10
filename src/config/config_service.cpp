#include <config/config_service.h>
#include <config/extractor.h>
#include <config/keys.h>

namespace modbus_gateway {

ConfigService::ConfigService(TracePath &tp, const nlohmann::json::value_type &data) {
  auto logLevelOpt = ExtractLogLevel(tp, data);
  if (logLevelOpt.has_value()) {
    logLevel = logLevelOpt.value();
  }

  auto threadsOpt = ExtractUnsignedNumberOpt<size_t>(tp, data, keys::threads);
  if (threadsOpt.has_value()) {
    threads = threadsOpt.value();
  }
}

}// namespace modbus_gateway
