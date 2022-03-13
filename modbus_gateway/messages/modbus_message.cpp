#include <messages/modbus_message.h>

namespace modbus_gateway
{

ModbusMessageInfo::ModbusMessageInfo( MessageId id, modbus::UnitId unit, exchange::ActorId master )
: messageId( id )
, unitId( unit )
, masterId( master )
{}

bool ModbusMessageInfo::operator==( const ModbusMessageInfo& rhs ) const
{
     return std::tie( messageId, unitId, masterId ) == std::tie( rhs.messageId, rhs.unitId, rhs.masterId );
}

bool ModbusMessageInfo::operator!=( const ModbusMessageInfo& rhs ) const
{
     return !( rhs == *this );
}

ModbusMessage::ModbusMessage( const ModbusMessageInfo& modbusMessageInfo, const ModbusBufferPtr& modbusBuffer )
: modbusMessageInfo_( modbusMessageInfo )
, modbusBuffer_( modbusBuffer )
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
