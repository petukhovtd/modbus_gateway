#pragma once

#include <config/config_types.h>

#include <common/logger.h>
#include <common/types_asio.h>

#include <modbus/modbus_types.h>

#include <optional>
#include <string>

namespace modbus_gateway {

std::optional<Logger::LogLevel> ConvertLogLevel(const std::string &logLevel);

std::optional<modbus::FrameType> ConvertFrameType(const std::string &frameType);

std::optional<asio::serial_port_base::parity::type> ConvertParity(const std::string &parity);

std::optional<asio::serial_port_base::stop_bits::type> ConvertStopBits(float stopBits);

std::optional<asio::serial_port_base::flow_control::type> ConvertFlowControl(const std::string &flowControl);

std::optional<NumericRangeType> ConvertNumericRangeType(const std::string &numericRangeType);

}// namespace modbus_gateway
