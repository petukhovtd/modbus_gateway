#pragma once

#include <string>

namespace modbus_gateway::keys {

// sections
const std::string service = "service";
const std::string slaves = "slaves";
const std::string masters = "masters";

// service
const std::string version = "version";
const std::string logLevel = "log_level";
const std::string threads = "threads";

// common
const std::string frame_type = "frame_type";
const std::string timeout = "timeout_ms";

// tcp
const std::string ipAddress = "ip_address";
const std::string ipPort = "ip_port";

// rtu
const std::string device = "device";
const std::string baud_rate = "baud_rate";
const std::string character_size = "character_size";
const std::string parity = "parity";
const std::string stop_bits = "stop_bits";
const std::string flow_control = "flow_control";

// unit id
const std::string unitIdSet = "unit_id";
const std::string numericRangeType = "type";
const std::string numericRangeValue = "value";
const std::string numericRangeBegin = "begin";
const std::string numericRangeEnd = "end";

}// namespace modbus_gateway::keys
