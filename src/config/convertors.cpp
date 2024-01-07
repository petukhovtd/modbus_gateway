#include <config/convertors.h>

namespace modbus_gateway {

std::optional<Logger::LogLevel> ConvertLogLevel(const std::string &logLevel) {
  if ("trace" == logLevel) {
    return Logger::LogLevel::Trace;
  }
  if ("debug" == logLevel) {
    return Logger::LogLevel::Debug;
  }
  if ("info" == logLevel) {
    return Logger::LogLevel::Info;
  }
  if ("warning" == logLevel) {
    return Logger::LogLevel::Warning;
  }
  if ("error" == logLevel) {
    return Logger::LogLevel::Error;
  }
  if ("critical" == logLevel) {
    return Logger::LogLevel::Critical;
  }
  return std::nullopt;
}

std::optional<modbus::FrameType> ConvertFrameType(const std::string &frameType) {
  if ("tcp" == frameType) {
    return modbus::TCP;
  }
  if ("rtu" == frameType) {
    return modbus::RTU;
  }
  if ("ascii" == frameType) {
    return modbus::ASCII;
  }
  return std::nullopt;
}

std::optional<asio::serial_port_base::parity::type> ConvertParity(const std::string &parity) {
  if ("none" == parity) {
    return asio::serial_port_base::parity::none;
  }
  if ("odd" == parity) {
    return asio::serial_port_base::parity::odd;
  }
  if ("even" == parity) {
    return asio::serial_port_base::parity::even;
  }
  return std::nullopt;
}

std::optional<asio::serial_port_base::stop_bits::type> ConvertStopBits(float stopBits) {
  if (1 == stopBits) {
    return asio::serial_port_base::stop_bits::one;
  }
  if (1.5 == stopBits) {
    return asio::serial_port_base::stop_bits::onepointfive;
  }
  if (2 == stopBits) {
    return asio::serial_port_base::stop_bits::two;
  }
  return std::nullopt;
}

std::optional<asio::serial_port_base::flow_control::type> ConvertFlowControl(const std::string &flowControl) {
  if ("none" == flowControl) {
    return asio::serial_port_base::flow_control::none;
  }
  if ("software" == flowControl) {
    return asio::serial_port_base::flow_control::software;
  }
  if ("hardware" == flowControl) {
    return asio::serial_port_base::flow_control::hardware;
  }
  return std::nullopt;
}
//std::optional< NumericRangeType > ConvertNumericRangeType( const std::string& numericRangeType )
//{
//     if( "range" == numericRangeType )
//     {
//          return NumericRangeType::Range;
//     }
//     if( "value" == numericRangeType )
//     {
//          return NumericRangeType::Value;
//     }
//     return std::nullopt;
//}

}// namespace modbus_gateway
