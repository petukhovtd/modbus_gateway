#pragma once

#include <common/types_modbus.h>
#include <message/modbus_message_info.h>

#include <exchange/message_helper.h>

namespace modbus_gateway {

class ModbusMessage
    : public exchange::MessageHelper<ModbusMessage> {
public:
  ModbusMessage(const ModbusMessageInfo &modbusMessageInfo, const ModbusBufferPtr &modbusBuffer);

  ~ModbusMessage() override = default;

  const ModbusMessageInfo &GetModbusMessageInfo() const;

  const ModbusBufferPtr &GetModbusBuffer() const;

private:
  ModbusMessageInfo modbusMessageInfo_;
  ModbusBufferPtr modbusBuffer_;
};

using ModbusMessagePtr = std::shared_ptr<ModbusMessage>;

}// namespace modbus_gateway
