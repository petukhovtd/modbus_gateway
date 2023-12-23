#ifndef MODBUS_GATEWAY_TRACE_PATH_H
#define MODBUS_GATEWAY_TRACE_PATH_H

#include <string>
#include <vector>

namespace modbus_gateway
{

class TracePath
{
public:
     TracePath() = default;
     TracePath( TracePath& ) = delete;

     void operator=( TracePath& ) = delete;

     void Push( std::string s );

     void Pop();

     std::string GetPath() const;

private:
     std::vector< std::string > path_;
};

}

#endif
