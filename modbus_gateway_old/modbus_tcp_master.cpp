#include "modbus_tcp_master.h"

#include <common/fmt_logger.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

#include <exchange/exchange.h>

namespace modbus_gateway
{

ModbusTcpMaster::ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, std::chrono::milliseconds timeout )
: socket_( std::make_unique< TcpSocketUPtr::element_type >( *context ) )
, ep_( addr, port )
, timeout_( timeout )
, timer_( socket_->get_executor() )
, m_()
, messageQueue_()
, currentMessage_( std::nullopt )
, transactionIdGenerator_( 0 )
, state_( State::Idle )
{
}

ModbusTcpMaster::~ModbusTcpMaster()
{
     asio::error_code ec;
     if( socket_->is_open() )
     {
          socket_->close( ec );
     }
}

void ModbusTcpMaster::Receive( const exchange::MessagePtr& message )
{
     FMT_LOG_TRACE( "ModbusTcpMaster::Receive" )
     auto modbusMessage = std::dynamic_pointer_cast< ModbusMessagePtr::element_type >( message );
     if( modbusMessage )
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::Receive ModbusMessage" )
          MessageProcess( modbusMessage );
          return;
     }
     FMT_LOG_TRACE( "ModbusTcpMaster::Receive unsupported message" )
}

void ModbusTcpMaster::MessageProcess( const ModbusMessagePtr& message )
{
     std::lock_guard< std::mutex > lock( m_ );
     messageQueue_.Push( message );
     FMT_LOG_TRACE( "MessageProcess: message in queue {}", messageQueue_.Size() )

     QueueProcessUnsafe();
}

void ModbusTcpMaster::QueueProcessUnsafe()
{
     FMT_LOG_TRACE( "ModbusTcpMaster::QueueProcess" )
     if( messageQueue_.Empty() )
     {
          FMT_LOG_DEBUG( "ModbusTcpMaster: queue empty" )
          return;
     }

     FMT_LOG_TRACE( "ModbusTcpMaster::QueueProcess: state: {}", StateToStr( state_ ) )

     switch( state_ )
     {
          case State::Idle:
               StartConnectTaskUnsafe();
               break;
          case State::WaitConnect:
               FMT_LOG_DEBUG( "MessageProcess: wait connect" )
               break;
          case State::Connected:
               StartMessageTaskUnsafe();
               break;
          case State::MessageProcess:
               FMT_LOG_DEBUG( "MessageProcess: message in process" )
               break;
     }
}

void ModbusTcpMaster::StartConnectTaskUnsafe()
{
     FMT_LOG_TRACE( "StartMessageTask::StartConnectTaskUnsafe: connect to {}:{}", ep_.address().to_string(), ep_.port() )
     Weak weak = GetWeak();
     socket_->async_connect( ep_, [ weak ]( asio::error_code ec )
     {
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "ModbusTcpMaster: connect: actor was deleted" )
               return;
          }

          std::lock_guard< std::mutex > lock( self->m_ );

          if( ec )
          {
               self->state_ = State::Idle;

               FMT_LOG_ERROR( "ModbusTcpMaster: connect: error: {}", ec.message() )
               if( ( asio::error::operation_aborted == ec ) )
               {
                    FMT_LOG_INFO( "ModbusTcpMaster: connect: canceled" )
                    return;
               }
               // Что бы не сыпать ошибки в лог при отстуствии сервера, подключаться будем только при новом сообщении
               //                    FMT_LOG_INFO( "ModbusTcpMaster: connect: start connect task" )
               //                    self->StartConnectTask();
               return;
          }

          self->state_ = State::Connected;

          FMT_LOG_INFO( "ModbusTcpMaster: connect to {}:{}",
                        self->socket_->remote_endpoint().address().to_string(),
                        self->socket_->remote_endpoint().port() )

          FMT_LOG_TRACE( "ModbusTcpMaster: connect: start queue process" )
          self->QueueProcessUnsafe();
     });
}

void ModbusTcpMaster::StartMessageTaskUnsafe()
{
     FMT_LOG_TRACE( "ModbusTcpMaster::StartMessageTaskUnsafe" )
     if( currentMessage_ )
     {
          FMT_LOG_INFO( "ModbusTcpMaster::StartMessageTaskUnsafe: message in process" )
          return;
     }
     while( !messageQueue_.Empty() )
     {
          const auto& message = messageQueue_.Front();
          if( !message->GetModbusMessageInfo().TimeoutReached( timeout_ ) )
          {
               break;
          }

          const auto& messageInfo = message->GetModbusMessageInfo();
          FMT_LOG_INFO(
          "ModbusTcpMaster::StartMessageTaskUnsafe: message reached timeout {}, message id {}, source id {}",
          timeout_.count(), messageInfo.GetTransactionId(), messageInfo.GetSourceId() )
          messageQueue_.Pop();
     }
     if( messageQueue_.Empty() )
     {
          FMT_LOG_INFO( "ModbusTcpMaster::StartMessageTaskUnsafe: message queue empty" )
          return;
     }

     state_ = State::MessageProcess;
     currentMessage_ = { messageQueue_.Front(), ++transactionIdGenerator_ };
     messageQueue_.Pop();

     ModbusBufferPtr modbusBuffer = currentMessage_->modbusMessage->GetModbusBuffer();
     modbusBuffer->ConvertTo( modbus::FrameType::TCP );
     {
          modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
          modbusBufferTcpWrapper.Update();
          const auto originTransactionId = modbusBufferTcpWrapper.GetTransactionId();
          modbusBufferTcpWrapper.SetTransactionId( currentMessage_->id );

          FMT_LOG_DEBUG( "ModbusTcpMaster::StartMessageTaskUnsafe: request: transaction id {}, "
                         "origin transaction id {}, "
                         "protocol id {}, "
                         "length {}, "
                         "unit id {}, "
                         "function code {}",
                         modbusBufferTcpWrapper.GetTransactionId(),
                         originTransactionId,
                         modbusBufferTcpWrapper.GetProtocolId(),
                         modbusBufferTcpWrapper.GetLength(),
                         modbusBuffer->GetUnitId(),
                         modbusBuffer->GetFunctionCode() )

          FMT_LOG_DEBUG( "ModbusTcpMaster::StartMessageTaskUnsafe: request: [{:X}]", fmt::join( *modbusBuffer, " " ) )
     }



     Weak weak = GetWeak();
     socket_->async_send( asio::buffer( modbusBuffer->begin().base(), modbusBuffer->GetAduSize() ), [ weak ]( asio::error_code ec, size_t size )
     {
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "ModbusTcpMaster: send: actor was deleted" )
               return;
          }

          std::lock_guard< std::mutex > lock( self->m_ );

          if( ec )
          {
               FMT_LOG_ERROR( "ModbusTcpMaster: send: error {}", ec.message() )
               self->currentMessage_.reset();
               if( asio::error::operation_aborted != ec )
               {
                    FMT_LOG_INFO( "ModbusTcpMaster: send: close connection" )
                    self->state_ = State::Idle;
                    self->socket_->close( ec );
               }
               return;
          }

          FMT_LOG_TRACE( "ModbusTcpMaster: send: start wait task" )
          self->StartWaitTask();
          FMT_LOG_TRACE( "ModbusTcpMaster: send: start receive task" )
          self->StartReceiveTask();
     });
}

void ModbusTcpMaster::StartWaitTask()
{
     FMT_LOG_TRACE( "ModbusTcpMaster::StartWaitTask" )

     FMT_LOG_DEBUG( "ModbusTcpMaster::StartWaitTask timeout {}ms", timeout_.count() )
     timer_.expires_after( timeout_ );

     Weak weak = GetWeak();
     timer_.async_wait( [ weak ]( asio::error_code ec )
                        {
                             Ptr self = weak.lock();
                             if( !self )
                             {
                                  FMT_LOG_CRIT( "ModbusTcpMaster: wait: actor was deleted" )
                                  return;
                             }

                             std::lock_guard< std::mutex > lock( self->m_ );

                             if( ec )
                             {
                                  if( asio::error::operation_aborted == ec )
                                  {
                                       FMT_LOG_TRACE( "ModbusTcpMaster: wait: canceled" )
                                       return;
                                  }
                                  FMT_LOG_ERROR( "ModbusTcpMaster: wait: error: {}", ec.message() )
                                  return;
                             }

                             FMT_LOG_TRACE( "ModbusTcpMaster: wait: achieve timeout, cancel receive task" )
                             self->socket_->cancel( ec );
                        });
}


ModbusMessagePtr ModbusTcpMaster::MakeResponse( const ModbusBufferPtr& modbusBuffer, size_t size )
{
     if( !currentMessage_ )
     {
          return nullptr;
     }

     FMT_LOG_DEBUG( "ModbusTcpMaster::MakeResponse: receive: {} bytes", size )
     if( !modbusBuffer->SetAduSize( size ) )
     {
          FMT_LOG_ERROR( "ModbusTcpMaster::MakeResponse: invalid adu size" )
          return nullptr;
     }

     FMT_LOG_TRACE( "ModbusTcpMaster::MakeResponse: response: [{:X}]", fmt::join( *modbusBuffer, " " ) )

     const auto currentMessage = currentMessage_->modbusMessage;
     const auto id = currentMessage_->id;
     currentMessage_.reset();

     const ModbusMessageInfo currentInfo = currentMessage->GetModbusMessageInfo();

     {
          modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
          const auto result = modbusBufferTcpWrapper.Check();
          if( result != modbus::CheckFrameResult::NoError )
          {
               FMT_LOG_ERROR( "ModbusTcpMaster: receive: check message failed: {}", result )
               return nullptr;
          }

          if( modbusBufferTcpWrapper.GetTransactionId() != id )
          {
               FMT_LOG_ERROR(
               "ModbusTcpMaster: receive: receive message id {} not equal send message id {}",
               modbusBufferTcpWrapper.GetTransactionId(), id )
               return nullptr;
          }
     }

     return ModbusMessage::Create( currentInfo, modbusBuffer );
}

void ModbusTcpMaster::StartReceiveTask()
{
     FMT_LOG_TRACE( "ModbusTcpMaster::StartReceiveTask" )
     // Теортерически возможно переиспользовать и входной буффер, но необходимо его
     // отчистить и выставить максимальный размер
     auto modbusBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     Weak weak = GetWeak();
     socket_->async_receive( asio::buffer( modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize() ),
                             [ weak, modbusBuffer ]( asio::error_code ec, size_t size )
                             {
                                  Ptr self = weak.lock();
                                  if( !self )
                                  {
                                       FMT_LOG_CRIT( "ModbusTcpMaster: receive: actor was deleted" )
                                       return;
                                  }

                                  std::lock_guard< std::mutex > lock( self->m_ );

                                  try
                                  {
                                       const auto tp = self->timer_.expiry() - std::chrono::steady_clock::now();
                                       const auto exp = tp.count() / std::chrono::microseconds::period::den;
                                       FMT_LOG_DEBUG("ModbusTcpMaster: receive: expiry {}", exp )
                                       self->timer_.cancel();
                                  }
                                  catch( const asio::system_error& e )
                                  {
                                       FMT_LOG_ERROR( "ModbusTcpMaster: timer cancel error: {}", ec.message() )
                                  }

                                  if( ec )
                                  {
                                       FMT_LOG_ERROR( "ModbusTcpMaster: receive: error: {}", ec.message() )
                                       self->currentMessage_.reset();
                                       if( asio::error::operation_aborted != ec )
                                       {
                                            FMT_LOG_INFO( "ModbusTcpMaster: receive: close connection" )
                                            self->state_ = State::Idle;
                                            self->socket_->close( ec );
                                            return;
                                       }
                                  }

                                  const auto modbusMessage = self->MakeResponse( modbusBuffer, size );
                                  if( modbusMessage )
                                  {
                                       exchange::Exchange::Send( modbusMessage->GetModbusMessageInfo().GetSourceId(),
                                                                 modbusMessage );
                                  }

                                  self->state_ = State::Connected;
                                  self->QueueProcessUnsafe();
                             });
}

std::string ModbusTcpMaster::StateToStr( State state )
{
     switch( state )
     {
          case State::Idle: return "Idle";
          case State::WaitConnect: return "WaitConnect";
          case State::Connected: return "Connected";
          case State::MessageProcess: return "MessageProcess";
          default: return "Unknown";
     }
}

}
