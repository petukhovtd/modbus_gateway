#pragma once

#include <config/parser_exception.h>
#include <config/value_type.h>

namespace modbus_gateway {

class InvalidTypeException : public ParserException {
public:
  explicit InvalidTypeException(const TraceDeep &traceDeep, ValueType expectType, ValueType actualType);

  ~InvalidTypeException() override = default;

  const char *what() const noexcept override;

  std::string GetExpectType() const;

  std::string GetActualType() const;

private:
  ValueType expectType_;
  ValueType actualType_;
};

}// namespace modbus_gateway
