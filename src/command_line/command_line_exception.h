#pragma once

#include <exception>
#include <string>

namespace modbus_gateway {

class CommandLineException : public std::exception {
public:
  explicit CommandLineException(const std::string &message) noexcept;

  ~CommandLineException() noexcept override = default;

  const char *what() const noexcept override;

private:
  std::string message_;
};

}// namespace modbus_gateway
