#pragma once

#include <config/trace_deep.h>
#include <modbus/modbus_types.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

struct UnitIdRange {
  UnitIdRange(TracePath &tracePath, const nlohmann::json::value_type &obj);

  void Normalize();

  modbus::UnitId begin{0};
  modbus::UnitId end{0};
};

}// namespace modbus_gateway
