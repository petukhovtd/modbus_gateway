#pragma once

#include <config/trace_path.h>
#include <config/unit_id_range.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <memory>

namespace modbus_gateway {

struct MasterConfig {
public:
  MasterConfig(TracePath &tracePath, const nlohmann::json::value_type &obj);

  virtual ~MasterConfig() = default;

  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);
  std::vector<UnitIdRange> unitIdSet{};
};

using ClientConfigPtr = std::shared_ptr<MasterConfig>;

}// namespace modbus_gateway
