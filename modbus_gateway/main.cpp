#include <common/fmt_logger.h>
#include <modbus_tcp_server.h>
#include <modbus_tcp_connection.h>

#include <modbus/modbus_buffer_tcp_wrapper.h>

#include <exchange/exchange.h>

#include <queue>
#include <iostream>

using namespace modbus_gateway;

class SingleRouter: public IRouter
{
public:
     explicit SingleRouter( exchange::ActorId id )
     : id_( id )
     {}

     ~SingleRouter() override = default;

     [[nodiscard]] exchange::ActorId Route( modbus::UnitId id ) const override
     {
          return id;
     }

private:
     exchange::ActorId id_;
};

template< typename T >
class LimitQueue
{
public:
     explicit LimitQueue( size_t maxSize )
     : queue_()
     , maxSize_( maxSize )
     {
          assert( maxSize_ > 0 );
     }

     LimitQueue()
     : LimitQueue( std::numeric_limits< size_t >::max() )
     {}

     void Push( const T& val )
     {
          if( queue_.size() > maxSize_ )
          {
               queue_.pop();
          }
          queue_.push( val );
     }

     void Pop()
     {
          return queue_.pop();
     }

     const T& Front() const
     {
          return queue_.front();
     }

     size_t Size() const
     {
          return queue_.size();
     }

     bool Empty() const
     {
          return queue_.empty();
     }

     size_t MaxSize() const
     {
          return maxSize_;
     }

private:
     std::queue< T > queue_;
     size_t maxSize_;
};

class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
{
     struct ModbusCurrentMessage
     {
          ModbusMessagePtr modbusMessage;
          modbus::TransactionId id;
     };

     enum class State
     {
          Idle,
          WaitConnect,
          Connected,
          MessageProcess,
     };

     static std::string StateToStr( State state )
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
public:
     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, std::chrono::milliseconds timeout )
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

     ~ModbusTcpMaster() override
     {
          asio::error_code ec;
          if( socket_->is_open() )
          {
               socket_->close( ec );
          }
     }

     void Receive( const exchange::MessagePtr& message ) override
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

private:
     void MessageProcess( const ModbusMessagePtr& message )
     {
          std::lock_guard< std::mutex > lock( m_ );
          messageQueue_.Push( message );
          FMT_LOG_TRACE( "MessageProcess: message in queue {}", messageQueue_.Size() )

          QueueProcessUnsafe();
     }

     void QueueProcessUnsafe()
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


     void StartConnectTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartConnectTask" )
          std::lock_guard< std::mutex > lock( m_ );
          StartConnectTaskUnsafe();
     }

     void StartConnectTaskUnsafe()
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

     void StartMessageTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartMessageTask" )
          std::lock_guard< std::mutex > lock( m_ );
          StartMessageTaskUnsafe();
     }

     void StartMessageTaskUnsafe()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartMessageTaskUnsafe" )
          if( messageQueue_.Empty() )
          {
               FMT_LOG_INFO( "StartMessageTaskUnsafe: message queue empty" )
               return;
          }
          if( currentMessage_ )
          {
               FMT_LOG_INFO( "StartMessageTaskUnsafe: message in process" )
               return;
          }

          state_ = State::MessageProcess;
          currentMessage_ = { messageQueue_.Front(), ++transactionIdGenerator_ };
          messageQueue_.Pop();

          ModbusBufferPtr modbusBuffer = currentMessage_->modbusMessage->GetModbusBuffer();
          {
               modbusBuffer->ConvertTo( modbus::FrameType::TCP );
               modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
               modbusBufferTcpWrapper.Update();
               modbusBufferTcpWrapper.SetTransactionId( currentMessage_->id );
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

     void StartWaitTask()
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

     void StartReceiveTask()
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
                                                 FMT_LOG_INFO( "ModbusTcpMaster: send: close connection" )
                                                 self->state_ = State::Idle;
                                                 self->socket_->close( ec );
                                            }
                                            return;
                                       }

                                       if( !self->currentMessage_ )
                                       {
                                            FMT_LOG_WARN( "ModbusTcpMaster: receive: no current message" )
                                            return;
                                       }

                                       FMT_LOG_DEBUG( "ModbusTcpMaster: receive: {} bytes", size )
                                       modbusBuffer->SetAduSize( size );

                                       const auto currentMessage = self->currentMessage_->modbusMessage;
                                       const auto id = self->currentMessage_->id;
                                       self->currentMessage_.reset();

                                       const ModbusMessageInfo currentInfo = currentMessage->GetModbusMessageInfo();

                                       {
                                             modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
                                             const auto result = modbusBufferTcpWrapper.Check();
                                             if( result != modbus::CheckFrameResult::NoError )
                                             {
                                                  FMT_LOG_ERROR( "ModbusTcpMaster: receive: check message failed: {}", result )
                                                  return;
                                             }

                                             if( modbusBufferTcpWrapper.GetTransactionId() != id )
                                             {
                                                  FMT_LOG_ERROR(
                                                  "ModbusTcpMaster: receive: receive message id {} not equal send message id {}",
                                                  modbusBufferTcpWrapper.GetTransactionId(), id )
                                                  return;
                                             }
                                       }

                                       exchange::Exchange::Send( currentInfo.GetSourceId(),
                                                                 ModbusMessage::Create( currentInfo, modbusBuffer ) );

                                       self->state_ = State::Connected;
                                       self->QueueProcessUnsafe();
                                  });
     }

private:
     TcpSocketUPtr socket_;
     asio::ip::tcp::endpoint ep_;
     TimeoutMs timeout_;
     asio::basic_waitable_timer< std::chrono::steady_clock > timer_;
     std::mutex m_;
     LimitQueue< ModbusMessagePtr > messageQueue_;
     std::optional< ModbusCurrentMessage > currentMessage_;
     modbus::TransactionId transactionIdGenerator_;
     State state_;
};

int main()
{
     using namespace std::chrono_literals;

     FmtLogger::SetLogLevel( FmtLogger::LogLevel::Trace );

     ContextPtr context = std::make_shared< ContextPtr::element_type >();
     auto work = asio::executor_work_guard( context->get_executor() );
     auto contextThread = std::thread( [ context ]()
                                       {
                                            context->run();
                                       } );

     ModbusTcpMaster::Ptr tcpMaster = ModbusTcpMaster::Create( context, asio::ip::make_address( "10.10.20.22"), 502, 1000ms );
     const auto tcpMasterId = exchange::Exchange::Insert( tcpMaster );

     RouterPtr router = std::make_shared< SingleRouter >( tcpMasterId );

     ModbusTcpServer::Ptr tcpServer = ModbusTcpServer::Create( context, asio::ip::address_v4::any(), 502, router );
     exchange::Exchange::Insert( tcpServer );
     tcpServer->Start();

     std::cin.get();

     tcpServer->Stop();

     context->stop();
     if( contextThread.joinable() )
     {
          contextThread.join();
     }

     return 0;
}