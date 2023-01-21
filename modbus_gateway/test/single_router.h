#ifndef MODBUS_GATEWAY_SINGLE_ROUTER_H
#define MODBUS_GATEWAY_SINGLE_ROUTER_H

#include <exchange/id.h>
#include <common/fmt_logger.h>
#include <i_router.h>

namespace test
{

class SingleRouter: public modbus_gateway::IRouter
{
public:
     explicit SingleRouter( exchange::ActorId id );

     ~SingleRouter() override = default;

     [[nodiscard]] exchange::ActorId Route( modbus::UnitId id ) const override;

private:
     exchange::ActorId id_;
};

}

#endif // MODBUS_GATEWAY_SINGLE_ROUTER_H
