#include <config/i_transport_config.h>

namespace modbus_gateway {

ITransportConfig::ITransportConfig(Type type)
    : type_(type) {}

ITransportConfig::Type ITransportConfig::GetType() const {
  return type_;
}

}// namespace modbus_gateway
