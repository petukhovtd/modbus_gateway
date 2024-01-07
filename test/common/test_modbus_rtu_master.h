#pragma once

#include <common/types_asio.h>
#include <common/types_modbus.h>

namespace test {

class TestModbusRtuMaster {
public:
  TestModbusRtuMaster(const modbus_gateway::ContextPtr &context, const std::string &device);

  void Cancel();

  void Process(const modbus_gateway::ModbusBufferPtr &sendModbusBuffer,
               const modbus_gateway::ModbusBufferPtr &receiveModbusBuffer);

private:
  asio::serial_port serialPort_;
};

}// namespace test
