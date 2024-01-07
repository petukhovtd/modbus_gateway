#pragma once

#include <exception>
#include <string>

namespace modbus_gateway {

class TraceDeep;

class ParserException : public std::exception {
public:
  explicit ParserException(const TraceDeep &traceDeep);

  ~ParserException() override = default;

  const std::string &GetFullPath() const;

  const std::string &GetTargetKey() const;

private:
  std::string path_;
  std::string key_;
};

}// namespace modbus_gateway
