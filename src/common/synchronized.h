#pragma once

#include <mutex>

namespace modbus_gateway {

template<typename T>
class Synchronized {
public:
  explicit Synchronized(T initial = T())
      : value(std::move(initial)) {}

  class Access {

  private:
    std::unique_lock<std::mutex> lock;

  public:
    T &ref;

    Access(std::mutex &m, T &value)
        : lock(m), ref(value) {}
  };

  Access GetAccess() {
    return Access(m, value);
  }

private:
  T value;
  std::mutex m;
};

}// namespace modbus_gateway
