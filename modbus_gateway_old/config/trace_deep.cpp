#include "trace_deep.h"
#include "trace_path.h"

namespace modbus_gateway
{

TraceDeep::TraceDeep( TracePath& tracePath, std::string key )
: tracePath_( tracePath )
, key_( std::move( key ) )
{
     tracePath_.Push( key_ );
}

TraceDeep::~TraceDeep()
{
     tracePath_.Pop();
}

TracePath& TraceDeep::GetTracePath()
{
     return tracePath_;
}

const TracePath& TraceDeep::GetTracePath() const
{
     return tracePath_;
}

const std::string& TraceDeep::GetKey() const
{
     return key_;
}

}
