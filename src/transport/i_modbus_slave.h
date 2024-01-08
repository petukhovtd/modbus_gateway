#pragma once

#include <transport/transport_type.h>

#include <memory>

namespace modbus_gateway {

class IModbusSlave {
public:
  explicit IModbusSlave(TransportType type);

  virtual ~IModbusSlave() = default;

  TransportType GetType() const;

  virtual void Start() = 0;

  virtual void Stop() = 0;

private:
  TransportType type_;
};

using ModbusSlavePtr = std::shared_ptr<IModbusSlave>;

}// namespace modbus_gateway
