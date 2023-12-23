#ifndef MODBUS_GATEWAY_PARSER_EXCEPTION_H
#define MODBUS_GATEWAY_PARSER_EXCEPTION_H

#include <exception>
#include <string>

namespace modbus_gateway
{

class TraceDeep;

class ParserException: public std::exception
{
public:
     explicit ParserException( const TraceDeep& traceDeep );

     ~ParserException() override = default;

     const std::string& GetFullPath() const;

     const std::string& GetTargetKey() const;

private:
     std::string path_;
     std::string key_;
};

}

#endif
