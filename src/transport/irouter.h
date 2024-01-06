#pragma once

#include <exchange/id.h>
#include <modbus/modbus_types.h>

#include <memory>

namespace modbus_gateway {

class IRouter {
public:
  virtual ~IRouter() = default;

  virtual exchange::ActorId Route(modbus::UnitId id) const = 0;
};

using RouterPtr = std::shared_ptr<IRouter>;

}// namespace modbus_gateway
