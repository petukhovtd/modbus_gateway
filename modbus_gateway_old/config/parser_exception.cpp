#include "parser_exception.h"
#include "trace_deep.h"
#include "trace_path.h"

namespace modbus_gateway
{
ParserException::ParserException( const TraceDeep& traceDeep )
: path_( traceDeep.GetTracePath().GetPath() )
, key_( traceDeep.GetKey() )
{}

const std::string& ParserException::GetFullPath() const
{
     return path_;
}

const std::string& ParserException::GetTargetKey() const
{
     return key_;
}

}