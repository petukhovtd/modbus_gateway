#pragma once

#include <common/types_asio.h>

#include <cstddef>

namespace test {

class ContextRunner {
public:
  explicit ContextRunner(size_t threads);

  ~ContextRunner();

  void Run();

  modbus_gateway::ContextPtr GetContext() const;

  void Stop();

private:
  modbus_gateway::ContextPtr context_;
  modbus_gateway::ContextWork work_;
  std::thread contextThread_;
};

}// namespace test

