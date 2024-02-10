#pragma once

#include <nlohmann/json.hpp>

namespace modbus_gateway {

enum ValueType {
  Unknown,
  Object,
  Array,
  String,
  Boolean,
  Number,
};

ValueType ConvertJsonTypeToValueType(nlohmann::json::value_t type);

std::string ConvertValueTypeToString(ValueType valueType);

}// namespace modbus_gateway
