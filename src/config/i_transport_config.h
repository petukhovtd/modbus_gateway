#pragma once

#include <memory>

namespace modbus_gateway {

class ITransportConfig {
public:
  enum class Type {
    TcpServer,
    RtuSlave,
    TcpClient,
    RtuMaster
  };
  explicit ITransportConfig(Type type);

  virtual ~ITransportConfig() = default;

  Type GetType() const;

protected:
  Type type_;
};

using TransportConfigPtr = std::shared_ptr<ITransportConfig>;

}// namespace modbus_gateway
