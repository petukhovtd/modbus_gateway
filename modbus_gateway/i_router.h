#ifndef MODBUS_GATEWAY_I_ROUTER_H
#define MODBUS_GATEWAY_I_ROUTER_H

#include <exchange/id.h>
#include <modbus/modbus_types.h>

namespace modbus_gateway
{

class IRouter
{
public:
     virtual ~IRouter() = default;

     virtual exchange::ActorId Route( modbus::UnitId id ) const = 0;
};

}

#endif
