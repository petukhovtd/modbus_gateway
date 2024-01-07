#pragma once

#include <config/i_transport_config.h>
#include <config/trace_path.h>

#include <transport/rtu_options.h>

#include <modbus/modbus_types.h>

#include "nlohmann/json.hpp"

#include <string>
#include <chrono>

namespace modbus_gateway {

struct RtuSlaveConfig: public ITransportConfig {

  RtuSlaveConfig(TracePath& tracePath, const nlohmann::json::value_type& obj,modbus::FrameType frameType);

  std::string device{};
  RtuOptions rtuOptions{};

  modbus::FrameType GetFrameType() const;

private:
  modbus::FrameType frameType_;

};

}// namespace modbus_gateway
