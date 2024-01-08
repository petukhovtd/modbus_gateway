#include <command_line/command_line_exception.h>

namespace modbus_gateway {

CommandLineException::CommandLineException(const std::string &message) noexcept
    : message_(message) {}

const char *CommandLineException::what() const noexcept {
  return message_.c_str();
}

}// namespace modbus_gateway