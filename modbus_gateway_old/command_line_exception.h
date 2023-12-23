#ifndef MODBUS_GATEWAY_COMMAND_LINE_EXCEPTION_H
#define MODBUS_GATEWAY_COMMAND_LINE_EXCEPTION_H

#include <exception>
#include <string>

namespace modbus_gateway
{

class CommandLineException: public std::exception
{
public:
     explicit CommandLineException( const std::string& message ) noexcept;

     ~CommandLineException() noexcept override = default;

     const char* what() const noexcept override;

private:
     std::string message_;
};

}

#endif //MODBUS_GATEWAY_COMMAND_LINE_EXCEPTION_H
