#include <config/trace_path.h>

#include <sstream>

namespace modbus_gateway {

void TracePath::Push(std::string s) {
  path_.push_back(std::move(s));
}

void TracePath::Pop() {
  if (!path_.empty()) {
    path_.pop_back();
  }
}

std::string TracePath::GetPath() const {
  std::ostringstream os;
  bool first = true;
  for (const auto &s : path_) {
    if (!first) {
      os << '.';
    } else {
      first = false;
    }
    os << s;
  }
  return os.str();
}

}// namespace modbus_gateway
