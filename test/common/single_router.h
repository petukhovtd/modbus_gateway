#pragma once

#include <exchange/id.h>
#include <modbus/modbus_types.h>
#include <transport/irouter.h>

namespace test {

class SingleRouter : public modbus_gateway::IRouter {
public:
    explicit SingleRouter(exchange::ActorId id);

    ~SingleRouter() override = default;

    exchange::ActorId Route(modbus::UnitId id) const override;

private:
    exchange::ActorId id_;
};

}
