#include <message/modbus_message.h>

namespace modbus_gateway {

ModbusMessage::ModbusMessage(const ModbusMessageInfo &modbusMessageInfo, const ModbusBufferPtr &modbusBuffer)
    : modbusMessageInfo_(modbusMessageInfo), modbusBuffer_(modbusBuffer) {}

const ModbusMessageInfo &ModbusMessage::GetModbusMessageInfo() const {
  return modbusMessageInfo_;
}

const ModbusBufferPtr &ModbusMessage::GetModbusBuffer() const {
  return modbusBuffer_;
}

}// namespace modbus_gateway
