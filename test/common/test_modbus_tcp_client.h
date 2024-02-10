#pragma once

#include <common/types_asio.h>
#include <common/types_modbus.h>

namespace test {

class TestModbusTcpClient {
public:
  TestModbusTcpClient(const modbus_gateway::ContextPtr &context, const asio::ip::address &addr,
                      asio::ip::port_type port);

  asio::error_code Connect();

  asio::error_code Disconnect();

  void Process(const modbus_gateway::ModbusBufferPtr &sendModbusBuffer,
               const modbus_gateway::ModbusBufferPtr &receiveModbusBuffer);

private:
  modbus_gateway::TcpSocketPtr socket_;
  modbus_gateway::TcpEndpoint ep_;
};

}// namespace test
