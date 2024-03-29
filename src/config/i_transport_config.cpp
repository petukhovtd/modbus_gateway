#include <config/i_transport_config.h>

namespace modbus_gateway {

ITransportConfig::ITransportConfig(TransportType type)
    : type_(type) {}

TransportType ITransportConfig::GetType() const {
  return type_;
}

}// namespace modbus_gateway
