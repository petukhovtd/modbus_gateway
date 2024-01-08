#pragma once

#include <common/types_asio.h>

namespace modbus_gateway {

struct RtuOptions {
  asio::serial_port_base::baud_rate baudRate{};
  asio::serial_port_base::character_size characterSize{};
  asio::serial_port_base::parity parity{};
  asio::serial_port_base::stop_bits stopBits{};
  asio::serial_port_base::flow_control flowControl{};
};

}// namespace modbus_gateway