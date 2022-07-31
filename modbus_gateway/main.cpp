#include <common/fmt_logger.h>
#include <modbus_tcp_server.h>
#include <modbus_tcp_connection.h>

#include <exchange/exchange.h>

#include <queue>

using namespace modbus_gateway;

class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
{
public:
     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, std::chrono::milliseconds timeout )
     : socket_( std::make_unique< TcpSocketUPtr::element_type >( *context ) )
     , ep_( addr, port )
     , timer_( socket_->get_executor(), timeout )
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
               socket_->cancel( ec );
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
          FMT_LOG_TRACE( "ModbusTcpMaster::StartConnectTask" )
          Weak weak = GetWeak();
          socket_->async_connect( ep_, [ weak ]( asio::error_code ec )
          {
               Ptr self = weak.lock();
               if( !self )
               {
                    FMT_LOG_CRIT( "ModbusTcpMaster: actor was deleted" )
                    return;
               }

               if( ( asio::error::eof == ec ) || ( asio::error::connection_reset == ec ) )
               {
                    FMT_LOG_INFO( "ModbusTcpMaster: disconnected: {}", ec.message() )
                    self->StartConnectTask();
                    return;
               }
               if( asio::error::operation_aborted == ec )
               {
                    FMT_LOG_INFO( "ModbusTcpMaster: canceled: {}", ec.message() )
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
                    FMT_LOG_CRIT( "ModbusTcpMaster: actor was deleted" )
                    return;
               }

               if( ( asio::error::eof == ec ) || ( asio::error::connection_reset == ec ) ||
                   ( asio::error::operation_aborted == ec ) )
               {
                    FMT_LOG_INFO( "ModbusTcpMaster: disconnected: {}", ec.message() )
                    return;
               }


          });
     }

     void StartWaitTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartWaitTask" )
          Weak weak = GetWeak();
          timer_.async_wait( [ weak ]( asio::error_code ec )
                             {

                             });
     }

private:
     TcpSocketUPtr socket_;
     asio::ip::tcp::endpoint ep_;
     asio::basic_waitable_timer< std::chrono::steady_clock > timer_;
     std::mutex m_;
     std::queue< ModbusMessagePtr > messageQueue_;
     ModbusMessagePtr currentMessage_;
};

int main()
{
//     TimeoutMs timeoutMs = 1000;
//     RouterPtr router = std::make_shared< Router >();
//     FmtLogger::SetLogLevel( FmtLogger::LogLevel::Trace );
//     ContextPtr context = std::make_shared< ContextPtr::element_type >();
//     TcpServer::Ptr tcpServer = TcpServer::Create( context, 1234, timeoutMs, router );
//     exchange::Exchange::Insert( tcpServer );
//     tcpServer->Start();
//     context->run();
     return 0;
}