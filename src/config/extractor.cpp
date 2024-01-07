#include <config/convertors.h>
#include <config/extractor.h>
#include <config/invalid_type_exception.h>
#include <config/keys.h>

#include <config/config_service.h>
#include <config/rtu_maser_config.h>
#include <config/rtu_slave_config.h>
#include <config/tcp_client_config.h>
#include <config/tcp_server_config.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

nlohmann::json::value_type::const_iterator FindObjectRaw(const TraceDeep &td, const nlohmann::json::value_type &obj) {
  return obj.find(td.GetKey());
}

const nlohmann::json::value_type &FindObject(const TraceDeep &td, const nlohmann::json::value_type &obj) {
  const auto it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    throw KeyNotFoundException(td);
  }
  return *it;
}

void CheckType(const TraceDeep &td, const nlohmann::json::value_type &obj, ValueType expectType) {
  const ValueType actualType = ConvertJsonTypeToValueType(obj.type());
  if (actualType != expectType) {
    throw InvalidTypeException(td, expectType, actualType);
  }
}

std::string ExtractString(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key) {
  TraceDeep td(tracePath, key);
  const auto &res = FindObject(td, obj);
  CheckType(td, res, ValueType::String);

  return res.get<std::string>();
}

std::optional<std::string> ExtractStringOpt(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key) {
  TraceDeep td(tracePath, key);
  const auto res = FindObjectRaw(td, obj);
  if (obj.end() == res) {
    return std::nullopt;
  }
  CheckType(td, *res, ValueType::String);

  return res->get<std::string>();
}

ConfigService ExtractConfigService(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::service);

  const auto &res = FindObject(td, obj);
  CheckType(td, res, ValueType::Object);

  return {td.GetTracePath(), res};
}

std::optional<Logger::LogLevel> ExtractLogLevel(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::logLevel);

  const auto it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    return std::nullopt;
  }
  const auto &val = it.value();
  CheckType(td, val, ValueType::String);

  const auto &value = val.get<std::string>();
  const auto convertValue = ConvertLogLevel(value);
  if (!convertValue.has_value()) {
    throw InvalidValueException(td, value);
  }
  return convertValue.value();
}

modbus::FrameType ExtractFrameType(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::frame_type);

  const auto &val = FindObject(td, obj);
  CheckType(td, val, ValueType::String);

  const auto &value = val.get<std::string>();
  const auto convertValue = ConvertFrameType(value);
  if (!convertValue.has_value()) {
    throw InvalidValueException(td, value);
  }
  return convertValue.value();
}

std::optional<asio::ip::address> ExtractIpAddressOptional(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::ipAddress);

  const auto it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    return std::nullopt;
  }
  const auto &val = it.value();
  CheckType(td, val, ValueType::String);

  const auto &value = val.get<std::string>();
  asio::error_code ec;
  const auto address = asio::ip::make_address_v4(value, ec);
  if (ec) {
    throw InvalidValueException(td, value);
  }
  return address;
}

asio::ip::address ExtractIpAddress(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  const auto &res = ExtractIpAddressOptional(tracePath, obj);
  if (!res.has_value()) {
    throw KeyNotFoundException(TraceDeep(tracePath, keys::ipAddress));
  }

  return res.value();
}

asio::ip::port_type ExtractIpPort(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::ipPort);

  const auto &val = FindObject(td, obj);
  CheckType(td, val, ValueType::Number);

  const auto value = val.get<uint64_t>();
  if (value == 0 || value > std::numeric_limits<asio::ip::port_type>::max()) {
    throw InvalidValueException(td, std::to_string(value));
  }

  return static_cast<asio::ip::port_type>(value);
}

RtuOptions ExtractRtuOptions(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  RtuOptions rtuOptions{};

  const auto baudRateOpt = ExtractUnsignedNumberOpt<size_t>(tracePath, obj, keys::baud_rate);
  if (baudRateOpt.has_value()) {
    rtuOptions.baudRate = asio::serial_port_base::baud_rate(baudRateOpt.value());
  }

  const auto characterSizeOpt = ExtractUnsignedNumberOpt<size_t>(tracePath, obj, keys::character_size);
  if (characterSizeOpt.has_value()) {
    rtuOptions.characterSize = asio::serial_port_base::character_size(characterSizeOpt.value());
  }

  const auto parityOptStr = ExtractStringOpt(tracePath, obj, keys::parity);
  if (parityOptStr.has_value()) {
    const auto partyOpt = ConvertParity(parityOptStr.value());
    if (!partyOpt.has_value()) {
      TraceDeep td(tracePath, keys::parity);
      throw InvalidValueException(td, parityOptStr.value());
    }
    rtuOptions.parity = asio::serial_port_base::parity(partyOpt.value());
  }

  const auto stopBitsNOpt = ExtractValueOpt<float>(tracePath, obj, keys::stop_bits, ValueType::Number);
  if (stopBitsNOpt.has_value()) {
    const auto stopBitsOpt = ConvertStopBits(stopBitsNOpt.value());
    if (!stopBitsOpt.has_value()) {
      TraceDeep td(tracePath, keys::stop_bits);
      throw InvalidValueException(td, std::to_string(stopBitsNOpt.value()));
    }
    rtuOptions.stopBits = asio::serial_port_base::stop_bits(stopBitsOpt.value());
  }

  const auto flowControlStrOpt = ExtractStringOpt(tracePath, obj, keys::flow_control);
  if (flowControlStrOpt.has_value()) {
    const auto flowControlOpt = ConvertFlowControl(flowControlStrOpt.value());
    if (!flowControlOpt.has_value()) {
      TraceDeep td(tracePath, keys::flow_control);
      throw InvalidValueException(td, flowControlStrOpt.value());
    }
    rtuOptions.flowControl = asio::serial_port_base::flow_control(flowControlOpt.value());
  }

  return rtuOptions;
}

NumericRangeType ExtractNumericRangeType(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::numericRangeType);

  const auto value = ExtractString(tracePath, obj, td.GetKey());
  const auto convertValue = ConvertNumericRangeType(value);
  if (!convertValue.has_value()) {
    throw InvalidValueException(td, value);
  }
  return convertValue.value();
}

modbus::UnitId ExtractModbusUnitId(TracePath &tracePath, const nlohmann::json::value_type &obj, const std::string &key) {
  const auto unitId = ExtractUnsignedNumber<modbus::UnitId>(tracePath, obj, key);
  if (unitId < modbus::unitIdMin || unitId > modbus::unitIdMax) {
    TraceDeep td(tracePath, key);
    throw InvalidValueException(td, std::to_string(unitId));
  }
  return unitId;
}

std::optional<std::vector<UnitIdRange>> ExtractUnitIdRangeSet(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::unitIdSet);

  const auto &it = FindObjectRaw(td, obj);
  if (obj.end() == it) {
    return std::nullopt;
  }
  const auto &unitIds = *it;
  CheckType(td, unitIds, ValueType::Array);

  if (unitIds.empty()) {
    throw InvalidValueException(td, keys::unitIdSet + " empty");
  }

  size_t index = 0;
  std::vector<UnitIdRange> result;
  result.reserve(unitIds.size());
  for (const auto &unitIdRange : unitIds) {
    TraceDeep tdUnitIdRange(td.GetTracePath(), std::to_string(index));
    result.emplace_back(tdUnitIdRange.GetTracePath(), unitIdRange);
    ++index;
  }

  return result;
}

TransportConfigPtr ExtractSlave(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  auto frameType = ExtractFrameType(tracePath, obj);
  switch (frameType) {
  case modbus::TCP:
    return std::make_shared<TcpServerConfig>(tracePath, obj);
  case modbus::RTU:
  case modbus::ASCII:
    return std::make_shared<RtuSlaveConfig>(tracePath, obj, frameType);
  default:
    throw std::logic_error("unimplemented frame type");
  }
}

std::vector<TransportConfigPtr> ExtractSlaves(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::slaves);

  const auto &slaves = FindObject(td, obj);
  CheckType(td, slaves, ValueType::Array);

  if (slaves.empty()) {
    throw InvalidValueException(td, keys::slaves + " empty");
  }

  size_t index = 0;
  std::vector<TransportConfigPtr> result;
  result.reserve(slaves.size());
  for (const auto &server : slaves) {
    TraceDeep tdServer(td.GetTracePath(), std::to_string(index));
    result.push_back(ExtractSlave(tdServer.GetTracePath(), server));
    ++index;
  }

  return result;
}

TransportConfigPtr ExtractMaster(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  auto frameType = ExtractFrameType(tracePath, obj);
  switch (frameType) {
  case modbus::TCP:
    return std::make_shared<TcpClientConfig>(tracePath, obj);
  case modbus::RTU:
  case modbus::ASCII:
    return std::make_shared<RtuMasterConfig>(tracePath, obj, frameType);
  default:
    throw std::logic_error("unimplemented frame type");
  }
}

std::vector<TransportConfigPtr> ExtractMasters(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  TraceDeep td(tracePath, keys::masters);

  const auto &servers = FindObject(td, obj);
  CheckType(td, servers, ValueType::Array);

  if (servers.empty()) {
    throw InvalidValueException(td, keys::masters + " empty");
  }

  size_t index = 0;
  std::vector<TransportConfigPtr> result;
  result.reserve(servers.size());
  for (const auto &server : servers) {
    TraceDeep tdServer(td.GetTracePath(), std::to_string(index));
    result.push_back(ExtractMaster(tdServer.GetTracePath(), server));
    ++index;
  }

  return result;
}

}// namespace modbus_gateway
