#pragma once

#include <common/types_asio.h>

#include <optional>

namespace modbus_gateway {

struct Rs485 {
  std::optional<bool> rtsOnSend{};
  std::optional<bool> rtsAfterSend{};
  std::optional<bool> rxDuringTx{};
  std::optional<bool> terminateBus{};
  std::optional<uint32_t> delayRtsBeforeSend{};
  std::optional<uint32_t> delayRtsAfterSend{};
};

struct RtuOptions {
  asio::serial_port_base::baud_rate baudRate{};
  asio::serial_port_base::character_size characterSize{};
  asio::serial_port_base::parity parity{};
  asio::serial_port_base::stop_bits stopBits{};
  asio::serial_port_base::flow_control flowControl{};
  std::optional<Rs485> rs485{};
};

void SetOptions( asio::serial_port& serialPort, const RtuOptions& options );

}// namespace modbus_gateway
