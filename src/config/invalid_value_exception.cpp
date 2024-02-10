#include <config/invalid_value_exception.h>

namespace modbus_gateway {

InvalidValueException::InvalidValueException(const TraceDeep &traceDeep, const std::string &value)
    : ParserException(traceDeep), value_(value) {
}

const char *InvalidValueException::what() const noexcept {
  return "invalid value";
}

const std::string &InvalidValueException::GetValue() const {
  return value_;
}

}// namespace modbus_gateway
