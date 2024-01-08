#pragma once

#include <transport/transport_type.h>

#include <memory>

namespace modbus_gateway {

class ITransportConfig {
public:

  explicit ITransportConfig(TransportType type);

  virtual ~ITransportConfig() = default;

  TransportType GetType() const;

protected:
  TransportType type_;
};

using TransportConfigPtr = std::shared_ptr<ITransportConfig>;

}// namespace modbus_gateway
