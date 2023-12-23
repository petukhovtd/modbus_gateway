#ifndef MODBUS_GATEWAY_TRACE_DEEP_H
#define MODBUS_GATEWAY_TRACE_DEEP_H

#include <string>

namespace modbus_gateway
{

class TracePath;

class TraceDeep
{
public:
     TraceDeep( TracePath& tracePath, std::string key );

     ~TraceDeep();

     TracePath& GetTracePath();

     const TracePath& GetTracePath() const;

     const std::string& GetKey() const;

private:
     TracePath& tracePath_;
     std::string key_;
};

}


#endif
