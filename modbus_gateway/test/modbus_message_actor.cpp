#include "modbus_message_actor.h"

#include <exchange/exchange.h>
#include <common/fmt_logger.h>

namespace test
{

ModbusMessageActor::ModbusMessageActor( ModbusMessageActor::Handler handler )
: handler_( std::move( handler ) )
{}

void ModbusMessageActor::Receive( const exchange::MessagePtr& message )
{
     const modbus_gateway::ModbusMessagePtr& modbusMessage = std::dynamic_pointer_cast< modbus_gateway::ModbusMessagePtr::element_type >(
     message );

     if( modbusMessage )
     {
          const exchange::ActorId sourceId = modbusMessage->GetModbusMessageInfo().GetSourceId();
          FMT_LOG_INFO( "ModbusMessageActor: process modbus message from {}", sourceId )
          modbus_gateway::ModbusMessagePtr response = handler_( modbusMessage );
          exchange::Exchange::Send( sourceId, response );
          return;
     }
     FMT_LOG_INFO( "ModbusMessageActor: unknown message type" )
}

}
