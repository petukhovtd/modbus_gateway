#ifndef MODBUS_GATEWAY_MODBUS_MESSAGE_H
#define MODBUS_GATEWAY_MODBUS_MESSAGE_H

#include <exchange/message_helper.h>

#include <types.h>
#include <modbus_message_info.h>

namespace modbus_gateway
{

class ModbusMessage
          : public exchange::MessageHelper< ModbusMessage >
{
public:
     ModbusMessage( const ModbusMessageInfo& modbusMessageInfo, ModbusBufferPtr  modbusBuffer );

     ~ModbusMessage() override = default;

     const ModbusMessageInfo& GetModbusMessageInfo() const;

     [[nodiscard]] const ModbusBufferPtr& GetModbusBuffer() const;

private:
     ModbusMessageInfo modbusMessageInfo_;
     ModbusBufferPtr modbusBuffer_;
};

using ModbusMessagePtr = std::shared_ptr< ModbusMessage >;

}

#endif
