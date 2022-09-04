#include "modbus_message_info.h"

namespace modbus_gateway
{

ModbusMessageInfo::ModbusMessageInfo( exchange::ActorId sourceId, modbus::TransactionId transactionId )
: createTimeStamp_( GetCurrentTimestamp() )
, timeoutReached_( false )
, sourceId_( sourceId )
, transactionId_( transactionId )
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

exchange::ActorId ModbusMessageInfo::GetSourceId() const
{
     return sourceId_;
}

modbus::TransactionId ModbusMessageInfo::GetTransactionId() const
{
     return transactionId_;
}

std::chrono::nanoseconds ModbusMessageInfo::GetCurrentTimestamp()
{
     return TargetClock::now().time_since_epoch();
}


}