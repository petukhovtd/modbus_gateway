#include <config/value_type.h>

namespace modbus_gateway {

ValueType ConvertJsonTypeToValueType(nlohmann::json::value_t type) {
  switch (type) {
  case nlohmann::json_abi_v3_11_2::detail::value_t::object:
    return ValueType::Object;
  case nlohmann::json_abi_v3_11_2::detail::value_t::array:
    return ValueType::Array;
  case nlohmann::json_abi_v3_11_2::detail::value_t::string:
    return ValueType::String;
  case nlohmann::json_abi_v3_11_2::detail::value_t::boolean:
    return ValueType::Boolean;
  case nlohmann::json_abi_v3_11_2::detail::value_t::number_integer:
  case nlohmann::json_abi_v3_11_2::detail::value_t::number_unsigned:
  case nlohmann::json_abi_v3_11_2::detail::value_t::number_float:
    return ValueType::Number;
  case nlohmann::json_abi_v3_11_2::detail::value_t::null:
  case nlohmann::json_abi_v3_11_2::detail::value_t::binary:
  case nlohmann::json_abi_v3_11_2::detail::value_t::discarded:
  default:
    return ValueType::Unknown;
  }
}

std::string ConvertValueTypeToString(ValueType valueType) {
  switch (valueType) {
  case Object:
    return "object";
  case Array:
    return "array";
  case String:
    return "string";
  case Boolean:
    return "boolean";
  case Number:
    return "number";
  case Unknown:
  default:
    return "unknown";
  }
}

}// namespace modbus_gateway