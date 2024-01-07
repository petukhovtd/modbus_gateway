#pragma once

#include <config/parser_exception.h>

namespace modbus_gateway {

class InvalidValueException : public ParserException {
public:
  explicit InvalidValueException(const TraceDeep &traceDeep, const std::string &value);

  ~InvalidValueException() override = default;

  const char *what() const noexcept override;

  const std::string &GetValue() const;

private:
  std::string value_;
};

}// namespace modbus_gateway
