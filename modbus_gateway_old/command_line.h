#ifndef MODBUS_GATEWAY_COMMAND_LINE_H
#define MODBUS_GATEWAY_COMMAND_LINE_H

#include <string>

#include <cxxopts.hpp>

namespace modbus_gateway
{
class CommandLine
{
public:
     CommandLine( int argc, char* argv[] );

     bool IsHelp() const;

     void PrintHelp( std::ostream& os ) const;

     const std::string& GetConfigPath() const;

private:
     cxxopts::Options options_;
     cxxopts::ParseResult parseResult_;
};
}

#endif //MODBUS_GATEWAY_COMMAND_LINE_H
