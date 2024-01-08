#include <transport/i_modbus_slave.h>

namespace modbus_gateway {

IModbusSlave::IModbusSlave(TransportType type)
    : type_(type) {}

TransportType IModbusSlave::GetType() const {
  return type_;
}

}// namespace modbus_gateway
