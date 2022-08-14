#include <common/fmt_logger.h>
#include <modbus_tcp_server.h>
#include <modbus_tcp_connection.h>

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

class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
{
public:
     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, std::chrono::milliseconds timeout )
     : socket_( std::make_unique< TcpSocketUPtr::element_type >( *context ) )
     , ep_( addr, port )
     , timeout_( timeout )
     , timer_( socket_->get_executor() )
     , m_()
     , messageQueue_()
     , currentMessage_( nullptr )
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
               FMT_LOG_TRACE( "ModbusTcpMaster::ModbusMessage" )
               MessageProcess( modbusMessage );
               return;
          }
          FMT_LOG_TRACE( "ModbusTcpMaster::Receive unsupported message" )
     }

private:
     void MessageProcess( const ModbusMessagePtr& message )
     {
          std::lock_guard< std::mutex > lock( m_ );
          messageQueue_.push( message );
          if( currentMessage_ )
          {
               FMT_LOG_DEBUG( "MessageProcess: message in queue {}", messageQueue_.size() )
               return;
          }

          if( socket_->is_open() )
          {
               StartMessageTaskUnsafe();
          }
          else
          {
               StartConnectTaskUnsafe();
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

               if( ec )
               {
                    FMT_LOG_ERROR( "ModbusTcpMaster: connect: error: {}", ec.message() )
                    self->currentMessage_.reset();
                    if( ( asio::error::operation_aborted == ec ) )
                    {
                         FMT_LOG_INFO( "ModbusTcpMaster: connect: canceled" )
                         return;
                    }
                    FMT_LOG_INFO( "ModbusTcpMaster: connect: start new connect tast" )
                    self->StartConnectTask();
                    return;
               }
               FMT_LOG_INFO( "ModbusTcpMaster: connect to {}:{}",
                             self->socket_->remote_endpoint().address().to_string(),
                             self->socket_->remote_endpoint().port() )
               self->StartMessageTask();
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
          if( !socket_->is_open() )
          {
               FMT_LOG_ERROR( "StartMessageTaskUnsafe: socket not open, start connect" )
               StartConnectTaskUnsafe();
               return;
          }
          if( messageQueue_.empty() )
          {
               FMT_LOG_ERROR( "StartMessageTaskUnsafe: message queue empty" )
               return;
          }
          if( currentMessage_ )
          {
               FMT_LOG_ERROR( "StartMessageTaskUnsafe: message in process" )
               return;
          }

          currentMessage_ = messageQueue_.front();
          messageQueue_.pop();

          const ModbusBufferPtr& modbusBufferPtr = currentMessage_->GetModbusBuffer();

          Weak weak = GetWeak();
          socket_->async_send( asio::buffer( modbusBufferPtr->begin().base(), modbusBufferPtr->GetAduSize() ), [ weak ]( asio::error_code ec, size_t size )
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
                    FMT_LOG_INFO( "ModbusTcpMaster: send: error {}", ec.message() )
                    self->currentMessage_.reset();
                    if( ( asio::error::eof == ec ) || ( asio::error::connection_reset == ec ) )
                    {
                         FMT_LOG_INFO( "ModbusTcpMaster: send: start new connect task" )
                         self->StartConnectTask();
                    }
                    return;
               }

               self->StartWaitTask();
               self->StartReceiveTask();
          });
     }

     void StartWaitTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartWaitTask" )
          Weak weak = GetWeak();
          timer_.expires_after( timeout_ );
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
                                  self->currentMessage_.reset();
                                  self->socket_->cancel( ec );
                             });
     }

     void StartReceiveTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartReceiveTask" )
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
                                            self->timer_.cancel();
                                       }
                                       catch( const asio::system_error& e )
                                       {
                                            FMT_LOG_ERROR( "ModbusTcpMaster: timer cancel error: {}", ec.message() )
                                       }

                                       if( ec )
                                       {
                                            if( asio::error::operation_aborted == ec )
                                            {
                                                 FMT_LOG_DEBUG( "ModbusTcpMaster: receive: canceled" )
                                                 return;
                                            }
                                            FMT_LOG_ERROR( "ModbusTcpMaster: receive: error: {}", ec.message() )
                                            self->currentMessage_.reset();
                                            return;
                                       }

                                       FMT_LOG_TRACE( "ModbusTcpMaster: receive: {} bytes", size )
                                       modbusBuffer->SetAduSize( size );

                                       const auto currentMessage = self->currentMessage_;
                                       self->currentMessage_.reset();

                                       const ModbusMessageInfo currentInfo = currentMessage->GetModbusMessageInfo();

                                       {
                                             // Check message here
                                       }

                                       exchange::Exchange::Send( currentInfo.GetSourceId(),
                                                                 ModbusMessage::Create( currentInfo, modbusBuffer ) );
                                  });
     }

private:
     TcpSocketUPtr socket_;
     asio::ip::tcp::endpoint ep_;
     TimeoutMs timeout_;
     asio::basic_waitable_timer< std::chrono::steady_clock > timer_;
     std::mutex m_;
     std::queue< ModbusMessagePtr > messageQueue_;
     ModbusMessagePtr currentMessage_;
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