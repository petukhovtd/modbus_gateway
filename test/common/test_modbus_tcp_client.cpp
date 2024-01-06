#include <common/test_modbus_tcp_client.h>

#include <common/logger.h>

namespace test {

TestModbusTcpClient::TestModbusTcpClient(const modbus_gateway::ContextPtr &context, const asio::ip::address &addr,
                                         asio::ip::port_type port)
        : socket_(std::make_unique<modbus_gateway::TcpSocketPtr::element_type>(*context)),
          ep_(asio::ip::address(addr), port) {}

asio::error_code TestModbusTcpClient::Connect() {
    asio::error_code ec;
    ec = socket_->connect(ep_, ec);
    return ec;
}

asio::error_code TestModbusTcpClient::Disconnect() {
    asio::error_code ec;
    ec = socket_->close(ec);
    return ec;
}

void TestModbusTcpClient::Process(const modbus_gateway::ModbusBufferPtr &sendModbusBuffer,
                                  const modbus_gateway::ModbusBufferPtr &receiveModbusBuffer) {
    socket_->async_send(asio::buffer(sendModbusBuffer->begin().base(), sendModbusBuffer->GetAduSize()),
                        [=](asio::error_code ec, size_t size) {
                            if (ec) {
                                MG_INFO("TestModbusTcpClient::send error: {}", ec.message())
                                return;
                            }
                            MG_INFO("TestModbusTcpClient::send {} bytes", size)
                            socket_->async_receive(
                                    asio::buffer(receiveModbusBuffer->begin().base(),
                                                 receiveModbusBuffer->GetAduSize()),
                                    [receiveModbusBuffer](asio::error_code ec, size_t size) {
                                        if (ec) {
                                            MG_INFO("TestModbusTcpClient::receive error: {}", ec.message())
                                            return;
                                        }
                                        MG_INFO("TestModbusTcpClient::receive {} bytes", size)
                                        receiveModbusBuffer->SetAduSize(size);
                                    });
                        });
}

}
