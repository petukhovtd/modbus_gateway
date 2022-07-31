#ifndef MODBUS_GATEWAY_MODBUS_MESSAGE_INFO_H
#define MODBUS_GATEWAY_MODBUS_MESSAGE_INFO_H

#include <exchange/id.h>

#include <chrono>

namespace modbus_gateway
{

using MessageId = uint64_t;

class ModbusMessageInfo
{
     using TargetClock = std::chrono::steady_clock;

public:
     explicit ModbusMessageInfo( MessageId messageId, exchange::ActorId sourceId );

     virtual ~ModbusMessageInfo() = default;

     bool TimeoutReached( std::chrono::nanoseconds timeout ) const;

     MessageId GetMessageId() const;

     exchange::ActorId GetSourceId() const;

private:
     static std::chrono::nanoseconds GetCurrentTimestamp();

private:
     std::chrono::nanoseconds createTimeStamp_;
     mutable bool timeoutReached_;
     MessageId messageId_;
     exchange::ActorId sourceId_;
};

}

#endif //MODBUS_GATEWAY_MODBUS_MESSAGE_INFO_H
