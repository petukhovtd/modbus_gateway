#pragma once

#include <string>
#include <vector>

namespace modbus_gateway {

class TracePath {
public:
  TracePath() = default;
  TracePath(TracePath &) = delete;

  void operator=(TracePath &) = delete;

  void Push(std::string s);

  void Pop();

  std::string GetPath() const;

private:
  std::vector<std::string> path_;
};

}// namespace modbus_gateway
