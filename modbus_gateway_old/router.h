#ifndef MODBUS_GATEWAY_ROUTER_H
#define MODBUS_GATEWAY_ROUTER_H

#include <config/config.h>

namespace modbus_gateway
{

class Router: public IRouter
{
public:
     explicit Router( exchange::ActorId defaultId );

     Router();

     ~Router() override = default;

     void Set( modbus::UnitId unitId, exchange::ActorId actorId );

     exchange::ActorId Route( modbus::UnitId unitId ) const override;

private:
     std::array< exchange::ActorId, std::numeric_limits< modbus::UnitId >::max() > routeMap_;
};

}

#endif //MODBUS_GATEWAY_ROUTER_H
