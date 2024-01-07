#include <common/test_modbus_tcp_server.h>

#include <common/logger.h>

namespace test {
TestModbusTcpServer::TestModbusTcpServer(const modbus_gateway::ContextPtr &context, const asio::ip::address &addr, asio::ip::port_type port)
    : acceptor_(*context, modbus_gateway::TcpEndpoint(addr, port)),
      socket_(nullptr),
      handler_([](const modbus_gateway::ModbusBufferPtr &in) { return in; }) {
}

void TestModbusTcpServer::SetHandler(const TestModbusTcpServer::Handler &handler) {
  handler_ = handler;
}

void TestModbusTcpServer::Start() {
  auto rawSocket = new modbus_gateway::TcpSocketPtr::element_type(acceptor_.get_executor());
  socket_ = std::unique_ptr<modbus_gateway::TcpSocketPtr::element_type>(rawSocket);
  acceptor_.async_accept(*rawSocket, [=](asio::error_code ec) {
    if (ec) {
      MG_ERROR("TestModbusTcpServer::accept: error: {}", ec.message());
      return;
    }
    MG_INFO("TestModbusTcpServer::accept: connect {}:{}",
            socket_->remote_endpoint().address().to_string(),
            socket_->remote_endpoint().port());
    StartReceive();
  });
}

void TestModbusTcpServer::StartReceive() {
  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  socket_->async_receive(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                         [=](asio::error_code ec, size_t size) mutable {
                           if (ec) {
                             MG_ERROR("TestModbusTcpServer::receive: error: {}", ec.message());
                             return;
                           }

                           MG_INFO("TestModbusTcpServer::receive: {} bytes", size);
                           modbusBuffer->SetAduSize(size);
                           auto result = handler_(modbusBuffer);
                           if (!result) {
                             StartReceive();
                             return;
                           }

                           socket_->async_send(asio::buffer(result->begin().operator->(), result->GetAduSize()),
                                               [=](asio::error_code ec, size_t size) {
                                                 if (ec) {
                                                   MG_ERROR("TestModbusTcpServer::send: error: {}", ec.message());
                                                   return;
                                                 }
                                                 MG_INFO("TestModbusTcpServer::send: {} bytes", size);
                                                 StartReceive();
                                               });
                         });
}

void TestModbusTcpServer::Stop() {
  Disconnect();
  asio::error_code ec;
  ec = acceptor_.cancel(ec);
  if (ec) {
    MG_WARN("TestModbusTcpServer::Stop acceptor cancel error: {}", ec.message());
  }
}

void TestModbusTcpServer::Disconnect() {
  asio::error_code ec;
  if (socket_->is_open()) {
    ec = socket_->cancel(ec);
    if (ec) {
      MG_WARN("TestModbusTcpServer::Stop socket cancel error: {}", ec.message());
    }
    socket_->shutdown(asio::socket_base::shutdown_both);
    ec = socket_->close(ec);
    if (ec) {
      MG_WARN("TestModbusTcpServer::Stop socket close error: {}", ec.message());
    }
  }
}

}// namespace test