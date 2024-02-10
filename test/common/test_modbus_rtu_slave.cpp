#include <common/test_modbus_rtu_slave.h>

#include <common/logger.h>

namespace test {
TestModbusRtuSlave::TestModbusRtuSlave(const modbus_gateway::ContextPtr &context, const std::string &device, modbus::FrameType frameType)
    : serialPort_(*context, device),
      handler_([](const modbus_gateway::ModbusBufferPtr &in) { return in; }),
      frameType_(frameType) {
}

void TestModbusRtuSlave::SetHandler(const TestModbusRtuSlave::Handler &handler) {
  handler_ = handler;
}

void TestModbusRtuSlave::Start() {
  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(frameType_);
  serialPort_.async_read_some(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                              [=](asio::error_code ec, size_t size) mutable {
                                if (ec) {
                                  MG_ERROR("TestModbusRtuSlave::read: error: {}", ec.message());
                                  return;
                                }

                                MG_INFO("TestModbusRtuSlave::read: {} bytes", size);
                                modbusBuffer->SetAduSize(size);
                                auto result = handler_(modbusBuffer);
                                if (!result) {
                                  Start();
                                  return;
                                }
                                serialPort_.async_write_some(asio::buffer(result->begin().operator->(), result->GetAduSize()),
                                                             [=](asio::error_code ec, size_t size) {
                                                               if (ec) {
                                                                 MG_ERROR("TestModbusRtuSlave::write: error: {}", ec.message());
                                                                 return;
                                                               }
                                                               MG_INFO("TestModbusRtuSlave::write: {} bytes", size);
                                                               Start();
                                                             });
                              });
}

void TestModbusRtuSlave::Stop() {
  asio::error_code ec;
  ec = serialPort_.cancel(ec);
  if (ec) {
    MG_WARN("TestModbusRtuSlave::Stop: serial port cancel error: {}", ec.message());
  }
}

}// namespace test