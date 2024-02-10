#include <config/parser_exception.h>
#include <config/trace_deep.h>
#include <config/trace_path.h>

namespace modbus_gateway {

ParserException::ParserException(const TraceDeep &traceDeep)
    : path_(traceDeep.GetTracePath().GetPath()), key_(traceDeep.GetKey()) {}

const std::string &ParserException::GetFullPath() const {
  return path_;
}

const std::string &ParserException::GetTargetKey() const {
  return key_;
}

}// namespace modbus_gateway