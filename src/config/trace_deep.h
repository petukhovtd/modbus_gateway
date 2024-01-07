#pragma once

#include <string>

namespace modbus_gateway {

class TracePath;

class TraceDeep {
public:
  TraceDeep(TracePath &tracePath, std::string key);

  TraceDeep(TraceDeep &) = delete;
  TraceDeep(TraceDeep &&) = delete;
  void operator=(TraceDeep &) = delete;
  void operator=(TraceDeep &&) = delete;

  ~TraceDeep();

  TracePath &GetTracePath();

  const TracePath &GetTracePath() const;

  const std::string &GetKey() const;

private:
  TracePath &tracePath_;
  std::string key_;
};

}// namespace modbus_gateway
