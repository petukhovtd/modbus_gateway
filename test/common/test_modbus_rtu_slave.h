#pragma once

#include <common/types_modbus.h>
#include <common/types_asio.h>

namespace test {

class TestModbusRtuSlave {
public:
  using Handler = std::function<modbus_gateway::ModbusBufferPtr(modbus_gateway::ModbusBufferPtr&)>;

  TestModbusRtuSlave(const modbus_gateway::ContextPtr &context, const std::string& device, modbus::FrameType type);

  void SetHandler( const Handler& handler);

  void Start();

  void Stop();

private:
  asio::serial_port serialPort_;
  Handler handler_;
  modbus::FrameType frameType_;
};

}// namespace test

