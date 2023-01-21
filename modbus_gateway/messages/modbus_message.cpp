#include "modbus_message.h"

namespace modbus_gateway
{

ModbusMessage::ModbusMessage( const ModbusMessageInfo& modbusMessageInfo, ModbusBufferPtr  modbusBuffer )
: modbusMessageInfo_( modbusMessageInfo )
, modbusBuffer_(std::move( modbusBuffer ))
{}

const ModbusMessageInfo& ModbusMessage::GetModbusMessageInfo() const
{
     return modbusMessageInfo_;
}

const ModbusBufferPtr& ModbusMessage::GetModbusBuffer() const
{
     return modbusBuffer_;
}

}
