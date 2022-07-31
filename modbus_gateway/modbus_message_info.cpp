#include "modbus_message_info.h"

namespace modbus_gateway
{

ModbusMessageInfo::ModbusMessageInfo( MessageId messageId, exchange::ActorId sourceId )
: createTimeStamp_( GetCurrentTimestamp() )
, timeoutReached_( false )
, messageId_( messageId )
, sourceId_( sourceId )
{}

bool ModbusMessageInfo::TimeoutReached( std::chrono::nanoseconds timeout ) const
{
     if( timeoutReached_ )
     {
          return true;
     }

     if( GetCurrentTimestamp() >= ( createTimeStamp_ + timeout ) )
     {
          timeoutReached_ = true;
     }

     return timeoutReached_;
}

MessageId ModbusMessageInfo::GetMessageId() const
{
     return messageId_;
}

exchange::ActorId ModbusMessageInfo::GetSourceId() const
{
     return sourceId_;
}

std::chrono::nanoseconds ModbusMessageInfo::GetCurrentTimestamp()
{
     return TargetClock::now().time_since_epoch();
}


}