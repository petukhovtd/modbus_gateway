#include <common/test_modbus_rtu_master.h>

#include <common/logger.h>

namespace test {
TestModbusRtuMaster::TestModbusRtuMaster(const modbus_gateway::ContextPtr &context, const std::string &device)
    : serialPort_(*context, device) {
  MG_TRACE("TestModbusRtuMaster::Ctor: open {}", device);
}

void TestModbusRtuMaster::Cancel() {
  asio::error_code ec;
  ec = serialPort_.cancel(ec);
  if (ec) {
    MG_WARN("TestModbusRtuMaster::Cancel: serial port cancel error: {}", ec.message());
  }
}

void TestModbusRtuMaster::Process(const modbus_gateway::ModbusBufferPtr &sendModbusBuffer, const modbus_gateway::ModbusBufferPtr &receiveModbusBuffer) {
  serialPort_.async_write_some(asio::buffer(sendModbusBuffer->begin().base(), sendModbusBuffer->GetAduSize()),
                               [=](asio::error_code ec, size_t size) {
                                 if (ec) {
                                   MG_ERROR("TestModbusRtuMaster::write: error: {}", ec.message());
                                   return;
                                 }
                                 MG_DEBUG("TestModbusRtuMaster::write: {} bytes", size);
                                 serialPort_.async_read_some(asio::buffer(receiveModbusBuffer->begin().base(), receiveModbusBuffer->GetAduSize()),
                                                             [=](asio::error_code ec, size_t size) {
                                                               if (ec) {
                                                                 MG_ERROR("TestModbusRtuMaster::read: error: {}", ec.message());
                                                                 return;
                                                               }
                                                               MG_DEBUG("TestModbusRtuMaster::read: {} bytes", size);
                                                               receiveModbusBuffer->SetAduSize(size);
                                                             });
                               });
}

}// namespace test