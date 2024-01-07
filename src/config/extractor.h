#pragma once

#include <config/i_transport_config.h>
#include <config/invalid_value_exception.h>
#include <config/key_not_found_exception.h>
#include <config/trace_deep.h>
#include <config/value_type.h>

#include <common/logger.h>
#include <common/types_asio.h>
#include <transport/rtu_options.h>

#include <modbus/modbus_types.h>

#include <nlohmann/json_fwd.hpp>

#include <optional>

namespace modbus_gateway {

class TraceDeep;
class TracePath;
class ConfigService;
//class UnitIdRange;

nlohmann::json::value_type::const_iterator FindObjectRaw(const TraceDeep &td, const nlohmann::json::value_type &obj);

const nlohmann::json::value_type &FindObject(const TraceDeep &td, const nlohmann::json::value_type &obj);

void CheckType(const TraceDeep &td, const nlohmann::json::value_type &obj, ValueType expectType);

template<typename T>
std::optional<T> ExtractValueOpt(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key, ValueType valueType) {
  TraceDeep td(tracePath, key);

  auto it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    return std::nullopt;
  }
  const auto &val = it.value();
  CheckType(td, val, valueType);

  return val.get<T>();
}

template<typename T>
std::optional<T> ExtractUnsignedNumberOpt(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key) {
  TraceDeep td(tracePath, key);

  auto it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    return std::nullopt;
  }
  const auto &val = it.value();
  CheckType(td, val, ValueType::Number);

  const auto value = val.get<uint64_t>();
  if (value > std::numeric_limits<T>::max()) {
    throw InvalidValueException(td, std::to_string(value));
  }

  return static_cast<T>(value);
}

template<typename T>
T ExtractUnsignedNumber(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key) {
  const auto resultOpt = ExtractUnsignedNumberOpt<T>(tracePath, obj, key);
  if (resultOpt.has_value()) {
    return resultOpt.value();
  }
  TraceDeep td(tracePath, key);
  throw KeyNotFoundException(td);
}

std::string ExtractString(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key);

std::optional<std::string> ExtractStringOpt(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key);

ConfigService ExtractConfigService(TracePath &tracePath, const nlohmann::json::value_type &obj);

std::optional<Logger::LogLevel> ExtractLogLevel(TracePath &tracePath, const nlohmann::json::value_type &obj);

modbus::FrameType ExtractFrameType(TracePath &tracePath, const nlohmann::json::value_type &obj);

std::optional<asio::ip::address> ExtractIpAddressOptional(TracePath &tracePath, const nlohmann::json::value_type &obj);

asio::ip::address ExtractIpAddress(TracePath &tracePath, const nlohmann::json::value_type &obj);

asio::ip::port_type ExtractIpPort(TracePath &tracePath, const nlohmann::json::value_type &obj);

RtuOptions ExtractRtuOptions(TracePath &tracePath, const nlohmann::json::value_type &obj);
//
//modbus::UnitId ExtractModbusUnitId( TracePath& tracePath, const nlohmann::json::value_type& obj, const std::string& key );
//
//NumericRangeType ExtractNumericRangeType( TracePath& tracePath, const nlohmann::json::value_type& obj );
//
//std::optional< std::vector< UnitIdRange > > ExtractUnitIdRangeSet( TracePath& tracePath, const nlohmann::json::value_type& obj );

TransportConfigPtr ExtractSlave(TracePath &tracePath, const nlohmann::json::value_type &obj);

std::vector<TransportConfigPtr> ExtractSlaves(TracePath &tracePath, const nlohmann::json::value_type &obj);

TransportConfigPtr ExtractMaster(TracePath &tracePath, const nlohmann::json::value_type &obj);

std::vector<TransportConfigPtr> ExtractMasters(TracePath &tracePath, const nlohmann::json::value_type &obj);

}// namespace modbus_gateway
