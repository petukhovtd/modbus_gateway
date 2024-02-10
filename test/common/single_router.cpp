#include <common/single_router.h>

#include <common/logger.h>

namespace test {

SingleRouter::SingleRouter(exchange::ActorId id)
        : id_(id) {}

exchange::ActorId SingleRouter::Route(modbus::UnitId id) const {
    MG_TRACE("Route: id {} to actorId {}", id, id_);
    return id_;
}

}
