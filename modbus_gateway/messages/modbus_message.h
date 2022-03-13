#ifndef MODBUS_GATEWAY_MODBUS_MESSAGE_H
#define MODBUS_GATEWAY_MODBUS_MESSAGE_H

#include <exchange/message_helper.h>
#include <types.h>

namespace modbus_gateway
{

using MessageId = uint64_t;

struct ModbusMessageInfo
{
     MessageId messageId;
     modbus::UnitId unitId;
     exchange::ActorId masterId;

     ModbusMessageInfo( MessageId messageId, modbus::UnitId unitId, exchange::ActorId masterId );

     bool operator==( const ModbusMessageInfo& rhs ) const;

     bool operator!=( const ModbusMessageInfo& rhs ) const;
};

class ModbusMessage
          : public exchange::MessageHelper< ModbusMessage >
{
public:
     ModbusMessage( const ModbusMessageInfo& modbusMessageInfo, const ModbusBufferPtr& modbusBuffer );

     ~ModbusMessage() override = default;

     const ModbusMessageInfo& GetModbusMessageInfo() const;

     const ModbusBufferPtr& GetModbusBuffer() const;

private:
     ModbusMessageInfo modbusMessageInfo_;
     ModbusBufferPtr modbusBuffer_;
};

using ModbusMessagePtr = std::shared_ptr< ModbusMessage >;

}

#endif
