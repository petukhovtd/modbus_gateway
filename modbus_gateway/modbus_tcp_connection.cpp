#include "modbus_tcp_connection.h"
#include <common/fmt_logger.h>
#include <messages/client_disconnect_message.h>
#include <messages/modbus_message.h>

#include <exchange/exchange.h>

#include <modbus/modbus_buffer.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

using namespace asio;

namespace modbus_gateway
{

ModbusTcpConnection::ModbusTcpConnection( exchange::ActorId serverId, TcpSocketPtr socket, RouterPtr  router )
: serverId_( serverId )
, socket_( std::move( socket ) )
, router_( std::move( router ) )
, syncRequestInfo_( std::nullopt )
{
     assert( socket_ );
     assert( router_ );
}

void ModbusTcpConnection::Receive( const exchange::MessagePtr& message )
{
     FMT_LOG_TRACE( "ModbusTcpConnection::Receive" )
     const ModbusMessagePtr& modbusMessage = std::dynamic_pointer_cast< ModbusMessagePtr::element_type >( message );
     if( modbusMessage )
     {
          FMT_LOG_TRACE( "ModbusTcpConnection::Receive ModbusMessage" )
          StartWriteMessage( modbusMessage );
          return;
     }
     FMT_LOG_TRACE( "ModbusTcpConnection::Receive unsupported message" )
}

void ModbusTcpConnection::Start()
{
     assert( GetId().has_value() );
     FMT_LOG_INFO( "ModbusTcpConnection::Start {}:{}, server id {}, client id {}",
                   socket_->remote_endpoint().address().to_string(),
                   socket_->remote_endpoint().port(), serverId_, GetId().value() )
     StartReadTask();
}

void ModbusTcpConnection::Stop()
{
     error_code ec;
     socket_->close( ec );
}

ModbusMessagePtr ModbusTcpConnection::MakeRequest( const ModbusBufferPtr& modbusBuffer, size_t size, exchange::ActorId masterId )
{
     if( !modbusBuffer->SetAduSize( size ) )
     {
          FMT_LOG_ERROR( "ModbusTcpConnection: invalid adu size" )
          return nullptr;
     }
     FMT_LOG_TRACE( "TcpClient: request: [{:X}]", fmt::join( *modbusBuffer, " " ) )
     modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
     modbus::CheckFrameResult checkFrameResult = modbusBufferTcpWrapper.Check();
     if( checkFrameResult != modbus::CheckFrameResult::NoError )
     {
          FMT_LOG_ERROR( "ModbusTcpConnection: invalid tcp frame {}", checkFrameResult )
          return nullptr;
     }

     FMT_LOG_DEBUG( "ModbusTcpConnection: request: transaction id {}, "
                    "protocol id {}, "
                    "length {}, "
                    "unit id {}, "
                    "function code {}",
                    modbusBufferTcpWrapper.GetTransactionId(),
                    modbusBufferTcpWrapper.GetProtocolId(),
                    modbusBufferTcpWrapper.GetLength(),
                    modbusBuffer->GetUnitId(),
                    modbusBuffer->GetFunctionCode() )

     ModbusMessageInfo modbusMessageInfo( modbusBufferTcpWrapper.GetTransactionId(), masterId );
     return std::make_shared< ModbusMessagePtr::element_type >( modbusMessageInfo, modbusBuffer );
}

void ModbusTcpConnection::StartReadTask()
{
     FMT_LOG_TRACE( "ModbusTcpConnection::StartReadTask" )
     Weak weak = GetWeak();
     auto modbusBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     socket_->async_receive( buffer( modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize() ),
               [ weak, modbusBuffer ]( error_code ec, size_t size )
               {
                    Ptr self = weak.lock();
                    if( !self )
                    {
                         FMT_LOG_CRIT( "ModbusTcpConnection: actor was deleted" )
                         return;
                    }

                    if( ec )
                    {
                         FMT_LOG_ERROR( "ModbusTcpConnection: receive error: {}", ec.message() )
                         if( ( error::eof == ec ) || ( error::connection_reset == ec ) ||
                             ( error::operation_aborted == ec ) )
                         {
                              FMT_LOG_INFO( "ModbusTcpConnection: send disconnect message" )
                              exchange::Exchange::Send( self->serverId_,
                                                        ClientDisconnectMessage::Create( self->GetId().value() ) );
                              return;
                         }

                         self->StartReadTask();
                         return;
                    }

                    FMT_LOG_TRACE( "ModbusTcpConnection: receive {} bytes", size )
                    auto message = self->MakeRequest( modbusBuffer, size, self->GetId().value() );
                    if( !message )
                    {
                         self->StartReadTask();
                         return;
                    }

                    {
                         auto access = self->syncRequestInfo_.GetAccess();
                         access.ref = message->GetModbusMessageInfo();
                    }

                    const modbus::UnitId unitId = message->GetModbusBuffer()->GetUnitId();
                    const exchange::ActorId slaveId = self->router_->Route( unitId );
                    FMT_LOG_TRACE( "ModbusTcpConnection: unit id {} route to slave id {}",
                                   message->GetModbusMessageInfo().GetSourceId(), slaveId )
                    const auto res = exchange::Exchange::Send( slaveId, message );
                    if( !res )
                    {
                         FMT_LOG_ERROR( "ModbusTcpConnection: route to slave id {} failed", slaveId )
                    }
               });
}

ModbusBufferPtr ModbusTcpConnection::MakeResponse( const ModbusMessagePtr& modbusMessage )
{
     const ModbusMessageInfo& messageInfo = modbusMessage->GetModbusMessageInfo();
     ModbusBufferPtr modbusBuffer = modbusMessage->GetModbusBuffer();

     if( GetId().value() != messageInfo.GetSourceId() )
     {
          FMT_LOG_WARN( "ModbusTcpConnection: receive stranger modbus message. self id {}, source id {}", GetId().value(),
                        messageInfo.GetSourceId() )
          return nullptr;
     }

     {
          auto access = syncRequestInfo_.GetAccess();
          ModbusMessageInfoOpt& modbusMessageInfoOpt = access.ref;
          if( !modbusMessageInfoOpt.has_value() )
          {
               FMT_LOG_ERROR( "ModbusTcpConnection: last message info is empty, unknown modbus message. message id {}",
                              messageInfo.GetMessageId() )
               return nullptr;
          }
          const ModbusMessageInfo& lastInfo = modbusMessageInfoOpt.value();
          if( lastInfo.GetMessageId() != messageInfo.GetMessageId() )
          {
               FMT_LOG_ERROR( "ModbusTcpConnection: receive message id {} not equal last message id {}",
                              messageInfo.GetMessageId(), lastInfo.GetMessageId() )
               return nullptr;
          }
          FMT_LOG_TRACE( "ModbusTcpConnection: message info checking succsessfully")
          modbusMessageInfoOpt.reset();
     }

     if( !modbusBuffer )
     {
          FMT_LOG_ERROR( "ModbusTcpConnection: modbus buffer is null. message id {}", messageInfo.GetMessageId() )
          return nullptr;
     }

     FMT_LOG_TRACE( "TcpClient: response: [{:X}]", fmt::join( *modbusBuffer, " " ) )

     modbusBuffer->ConvertTo( modbus::FrameType::TCP );
     modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
     modbusBufferTcpWrapper.Update();
     modbusBufferTcpWrapper.SetTransactionId( messageInfo.GetMessageId() );


     FMT_LOG_TRACE( "ModbusTcpConnection: tcp response: transaction id {}, "
                    "protocol id {}, "
                    "length {}, "
                    "unit id {}, "
                    "function code {}",
                    modbusBufferTcpWrapper.GetTransactionId(),
                    modbusBufferTcpWrapper.GetProtocolId(),
                    modbusBufferTcpWrapper.GetLength(),
                    modbusBuffer->GetUnitId(),
                    modbusBuffer->GetFunctionCode() )

     return modbusBuffer;
}

void ModbusTcpConnection::StartWriteMessage( const ModbusMessagePtr& modbusMessage )
{
     FMT_LOG_TRACE( "ModbusTcpConnection::SyncWriteMessage" )
     ModbusBufferPtr modbusBuffer = MakeResponse( modbusMessage );
     if( !modbusBuffer )
     {
          return;
     }

     Weak weak = GetWeak();
     socket_->async_send( buffer( modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize() ),
                          [ weak, modbusBuffer ]( error_code ec, size_t size )
                          {
                               Ptr self = weak.lock();
                               if( !self )
                               {
                                    FMT_LOG_CRIT( "ModbusTcpConnection: actor was deleted" )
                                    return;
                               }

                               if( ec )
                               {
                                    FMT_LOG_ERROR( "ModbusTcpConnection: write error: {}", ec.message() )
                                    return;
                               }

                               FMT_LOG_TRACE( "ModbusTcpConnection: send {} bytes", size )
                          } );
}

}
