#include <transport/router.h>

namespace modbus_gateway {

Router::Router(exchange::ActorId defaultId)
    : routeMap_() {
  routeMap_.fill(defaultId);
}

Router::Router()
    : Router(exchange::defaultId) {}

exchange::ActorId Router::Route(modbus::UnitId unitId) const {
  return routeMap_[unitId];
}

void Router::Set(modbus::UnitId unitId, exchange::ActorId actorId) {
  routeMap_[unitId] = actorId;
}

}// namespace modbus_gateway
