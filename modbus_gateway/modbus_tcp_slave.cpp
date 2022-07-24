#include <modbus_tcp_slave.h>
#include <common/fmt_logger.h>
#include <messages/client_disconnect_message.h>
#include <messages/modbus_message.h>

#include <exchange/exchange.h>

#include <modbus/modbus_buffer.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>
#include <zconf.h>

using namespace asio;

namespace modbus_gateway
{

ModbusTcpSlave::ModbusTcpSlave( exchange::ActorId serverId, TcpSocketPtr socket, std::chrono::milliseconds requestTimeout, const RouterPtr& router )
: serverId_( serverId )
, socket_( std::move( socket ) )
, requestTmeout_( requestTimeout )
, requestTimer_( std::make_unique< WaitTimerPtr::element_type >( socket_->get_executor() ) )
, router_( router )
, syncRequestInfo_( std::nullopt )
{
     assert( socket_ );
     assert( requestTimer_ );
     assert( router_ );
}

void ModbusTcpSlave::Receive( const exchange::MessagePtr& message )
{
     FMT_LOG_TRACE( "TcpClient::Receive" )
     auto targetMessage = dynamic_cast< ModbusMessage* >( message.get() );
     if( targetMessage )
     {
          FMT_LOG_TRACE( "TcpClient::Receive ModbusMessage" )
          SyncWriteMessage( *targetMessage );
          return;
     }
     FMT_LOG_TRACE( "TcpClient::Receive unsupported message" )
}

void ModbusTcpSlave::Start()
{
     assert( GetId().has_value() );
     FMT_LOG_INFO( "TcpClient::Start {}:{}, server id {}, client id {}",
                   socket_->remote_endpoint().address().to_string(),
                   socket_->remote_endpoint().port(), serverId_, GetId().value() )
     StartReadTask();
}

void ModbusTcpSlave::Stop()
{
     error_code ec;
     requestTimer_->cancel();
     socket_->cancel( ec );
}

ModbusMessagePtr ModbusTcpSlave::MakeRequest( const ModbusBufferPtr& modbusBuffer, size_t size, exchange::ActorId masterId )
{
     if( !modbusBuffer->SetAduSize( size ) )
     {
          FMT_LOG_ERROR( "TcpClient: invalid adu size" )
          return nullptr;
     }
     FMT_LOG_TRACE( "TcpClient: request: [{:X}]", fmt::join( *modbusBuffer, " " ) )
     modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
     modbus::CheckFrameResult checkFrameResult = modbusBufferTcpWrapper.Check();
     if( checkFrameResult != modbus::CheckFrameResult::NoError )
     {
          FMT_LOG_ERROR( "TcpClient: invalid tcp frame {}", checkFrameResult )
          return nullptr;
     }

     FMT_LOG_DEBUG( "TcpClient: request: transaction id {}, "
                    "protocol id {}, "
                    "length {}, "
                    "unit id {}, "
                    "function code {}",
                    modbusBufferTcpWrapper.GetTransactionId(),
                    modbusBufferTcpWrapper.GetProtocolId(),
                    modbusBufferTcpWrapper.GetLength(),
                    modbusBuffer->GetUnitId(),
                    modbusBuffer->GetFunctionCode() )

     ModbusMessageInfo modbusMessageInfo( modbusBufferTcpWrapper.GetTransactionId(), modbusBuffer->GetUnitId(), masterId );
     return std::make_shared< ModbusMessagePtr::element_type >( modbusMessageInfo, modbusBuffer );
}

void ModbusTcpSlave::StartReadTask()
{
     FMT_LOG_TRACE( "TcpClient::StartReadTask" )
     Weak weak = GetWeak();
     auto modbusBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     socket_->async_receive( buffer( modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize() ),
               [ weak, modbusBuffer ]( error_code ec, size_t size )
               {
                    Ptr self = weak.lock();
                    if( !self )
                    {
                         FMT_LOG_CRIT( "TcpClient: actor was deleted" )
                         return;
                    }

                    if( ( error::eof == ec ) || ( error::connection_reset == ec ) )
                    {
                         FMT_LOG_INFO( "TcpClient: client disconnect: {}", ec.message() )

                         exchange::Exchange::Send( self->serverId_,
                                                   ClientDisconnectMessage::Create( self->GetId().value() ) );
                         return;
                    }

                    if( ec )
                    {
                         FMT_LOG_ERROR( "TcpClient: read error: {}", ec.message() )
                         self->StartReadTask();
                         return;
                    }

                    FMT_LOG_TRACE( "TcpClient: read {} bytes", size )
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
                    self->StartTimeoutTask();
                    const exchange::ActorId slaveId = self->router_->Route( message->GetModbusMessageInfo().unitId );
                    FMT_LOG_TRACE( "TcpClient: unit id {} route to slave id {}", message->GetModbusMessageInfo().unitId, slaveId )
                    const auto res = exchange::Exchange::Send( slaveId, message );
                    if( !res )
                    {
                         FMT_LOG_ERROR( "TcpClient: route to slave id {} failed", slaveId )
                    }
               });
}

void ModbusTcpSlave::StartTimeoutTask()
{
     FMT_LOG_TRACE( "TcpClient::StartTimeoutTask" )
     Weak weak = GetWeak();
     requestTimer_->expires_after( requestTmeout_ );
     FMT_LOG_TRACE( "TcpClient: timeout {}ms", requestTmeout_.count() )
     requestTimer_->async_wait( [ weak ]( const error_code ec )
     {
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "TcpClient::Timer actor was deleted" )
               return;
          }
          if( ec == error::operation_aborted )
          {
               FMT_LOG_DEBUG( "TcpClient::Timer cancel" )
               return;
          }

          {
               auto access = self->syncRequestInfo_.GetAccess();
               ModbusMessageInfoOpt& modbusMessageInfoOpt = access.ref;
               if( !modbusMessageInfoOpt.has_value() )
               {
                    FMT_LOG_ERROR( "TcpClient::Timer empty request info" )
                    return;
               }
               FMT_LOG_INFO( "TcpClient::Timer achieve deadline. Message id {}, unit id {}",
                             modbusMessageInfoOpt->messageId, modbusMessageInfoOpt->unitId )
               modbusMessageInfoOpt.reset();
          }
          self->StartReadTask();
     } );
}

ModbusBufferPtr ModbusTcpSlave::MakeResponse( const ModbusMessage& modbusMessage )
{
     const ModbusMessageInfo& messageInfo = modbusMessage.GetModbusMessageInfo();
     ModbusBufferPtr modbusBuffer = modbusMessage.GetModbusBuffer();

     if( !modbusBuffer )
     {
          FMT_LOG_ERROR( "TcpClient: modbus buffer is null. Response message id {}, unit id {}",
                         messageInfo.messageId, messageInfo.unitId )
          return modbusBuffer;
     }

     FMT_LOG_TRACE( "TcpClient: response: [{:X}]", fmt::join( *modbusBuffer, " " ) )

     if( modbusBuffer->GetUnitId() != messageInfo.unitId )
     {
          FMT_LOG_ERROR( "TcpClient: response info and buffer unit id don't equal. "
                         "Info message id {}, unit id {}, buffer unit id {}",
                         messageInfo.messageId, messageInfo.unitId, modbusBuffer->GetUnitId() )
          return nullptr;
     }

     {
          auto access = syncRequestInfo_.GetAccess();
          ModbusMessageInfoOpt& modbusMessageInfoOpt = access.ref;
          if( !modbusMessageInfoOpt.has_value() )
          {
               FMT_LOG_ERROR( "TcpClient: empty request info. Response message id {}, unit id {}",
                              messageInfo.messageId, messageInfo.unitId )
               return nullptr;
          }
          const ModbusMessageInfo& lastInfo = modbusMessageInfoOpt.value();
          if( lastInfo != messageInfo )
          {
               FMT_LOG_ERROR( "TcpClient: request info don't equal response info. "
                              "Request message id {}, unit id {}, master id {}. "
                              "Response message id {}, unit id {}, master id {}",
                              lastInfo.messageId, lastInfo.unitId, lastInfo.masterId,
                              messageInfo.messageId, messageInfo.unitId, messageInfo.masterId )
               return nullptr;
          }
          modbusMessageInfoOpt.reset();
     }

     modbusBuffer->ConvertTo( modbus::FrameType::TCP );
     modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
     modbusBufferTcpWrapper.Update();
     modbusBufferTcpWrapper.SetTransactionId( messageInfo.messageId );


     FMT_LOG_DEBUG( "TcpClient: tcp response: transaction id {}, "
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

void ModbusTcpSlave::SyncWriteMessage( const ModbusMessage& modbusMessage )
{
     FMT_LOG_TRACE( "TcpClient::SyncWriteMessage" )
     ModbusBufferPtr modbusBuffer = MakeResponse( modbusMessage );
     if( !modbusBuffer )
     {
          return;
     }

     requestTimer_->cancel();

     error_code ec;
     size_t bytes = socket_->write_some( buffer( modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize() ), ec );

     if( ( error::eof == ec ) || ( error::connection_reset == ec ) )
     {
          FMT_LOG_INFO( "TcpClient: client disconnect: {}", ec.message() )
          exchange::Exchange::Send( serverId_,ClientDisconnectMessage::Create( GetId().value() ) );
          return;
     }

     if( ec )
     {
          FMT_LOG_ERROR( "TcpClient: write error: {}", ec.message() )
     }
     else
     {
          FMT_LOG_TRACE( "TcpClient: write {}", bytes )
     }

     StartReadTask();
}


}
