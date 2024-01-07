#pragma once

#include <config/i_transport_config.h>
#include <config/trace_path.h>
#include <config/unit_id_range.h>

#include <transport/rtu_options.h>

#include <modbus/modbus_types.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

struct RtuMasterConfig : public ITransportConfig {

  RtuMasterConfig(TracePath &tracePath, const nlohmann::json::value_type &obj, modbus::FrameType frameType);

  ~RtuMasterConfig() override = default;

  modbus::FrameType GetFrameType() const;

  std::string device{};
  RtuOptions rtuOptions{};
  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);
  std::vector<UnitIdRange> unitIdSet{};

private:
  modbus::FrameType frameType_;
};

}// namespace modbus_gateway
