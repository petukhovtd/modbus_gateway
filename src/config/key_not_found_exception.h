#pragma once

#include <config/parser_exception.h>

namespace modbus_gateway {

class TraceDeep;

class KeyNotFoundException : public ParserException {
public:
  explicit KeyNotFoundException(const TraceDeep &traceDeep);

  ~KeyNotFoundException() override = default;

  const char *what() const noexcept override;
};

}// namespace modbus_gateway
