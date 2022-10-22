#ifndef MODBUS_GATEWAY_INVALID_TYPE_EXCEPTION_H
#define MODBUS_GATEWAY_INVALID_TYPE_EXCEPTION_H

#include "parser_exception.h"
#include "value_type.h"

namespace modbus_gateway
{

class InvalidTypeException: public ParserException
{
public:
     explicit InvalidTypeException( const TraceDeep& traceDeep, ValueType expectType, ValueType actualType );

     ~InvalidTypeException() override = default;

     const char* what() const noexcept override;

     std::string GetExpectType() const;

     std::string GetActualType() const;

private:
     ValueType expectType_;
     ValueType actualType_;
};


}



#endif
