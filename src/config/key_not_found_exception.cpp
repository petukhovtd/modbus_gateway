#include <config/key_not_found_exception.h>

namespace modbus_gateway {

KeyNotFoundException::KeyNotFoundException(const TraceDeep &traceDeep)
    : ParserException(traceDeep) {}

const char *KeyNotFoundException::what() const noexcept {
  return "key not found";
}

}// namespace modbus_gateway
