#include "single_router.h"

namespace test
{

SingleRouter::SingleRouter( exchange::ActorId id )
: id_( id )
{}

exchange::ActorId SingleRouter::Route( modbus::UnitId id ) const
{
     FMT_LOG_TRACE( "Route: id {}", id )
     return id_;
}

}
