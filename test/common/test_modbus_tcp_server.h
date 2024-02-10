#pragma once

#include <common/types_asio.h>
#include <common/types_modbus.h>

namespace test {

class TestModbusTcpServer {
public:
  using Handler = std::function<modbus_gateway::ModbusBufferPtr(modbus_gateway::ModbusBufferPtr&)>;

  TestModbusTcpServer(const modbus_gateway::ContextPtr &context, const asio::ip::address &addr, asio::ip::port_type port);

  ~TestModbusTcpServer() = default;

  void SetHandler(const Handler& handler);

  void Start();

  void Stop();

  void Disconnect();

private:
  void StartReceive();

private:
  modbus_gateway::TcpAcceptor acceptor_;
  modbus_gateway::TcpSocketPtr socket_;
  Handler handler_;
};

}// namespace test
