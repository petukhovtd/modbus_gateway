#include "invalid_type_exception.h"

namespace modbus_gateway
{

InvalidTypeException::InvalidTypeException( const TraceDeep& traceDeep, ValueType expectType, ValueType actualType )
: ParserException( traceDeep )
, expectType_( expectType )
, actualType_( actualType )
{}

const char* InvalidTypeException::what() const noexcept
{
     return "invalid type";
}

std::string InvalidTypeException::GetExpectType() const
{
     return ConvertValueTypeToString( expectType_ );
}

std::string InvalidTypeException::GetActualType() const
{
     return ConvertValueTypeToString( actualType_ );
}

}