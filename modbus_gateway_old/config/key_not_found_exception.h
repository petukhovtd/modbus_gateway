#ifndef MODBUS_GATEWAY_KEY_NOT_FOUND_EXCEPTION_H
#define MODBUS_GATEWAY_KEY_NOT_FOUND_EXCEPTION_H

#include "parser_exception.h"

namespace modbus_gateway
{

class TraceDeep;

class KeyNotFoundException: public ParserException
{
public:
     explicit KeyNotFoundException( const TraceDeep& traceDeep );

     ~KeyNotFoundException() override = default;

     const char* what() const noexcept override;
};

}

#endif
