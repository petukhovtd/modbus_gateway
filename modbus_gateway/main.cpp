#include <common/fmt_logger.h>
#include <modbus_tcp_server.h>
#include <modbus_tcp_connection.h>

#include <exchange/exchange.h>

using namespace modbus_gateway;

//class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
//{
//public:
//     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port )
//     : socket_( std::make_unique< TcpSocketUPtr::element_type >( *context ) )
//     , ep_( addr, port )
//     , m_()
//     , messageQueue_()
//     , currentMessage_( nullptr )
//     {
//     }
//
//     ~ModbusTcpMaster() override
//     {
//          asio::error_code ec;
//          if( socket_->is_open() )
//          {
//               socket_->cancel( ec );
//               socket_->close( ec );
//          }
//     }
//
//     void Receive( const exchange::MessagePtr& message ) override
//     {
//          FMT_LOG_TRACE( "ModbusTcpMaster::Receive" )
//          auto modbusMessage = std::dynamic_pointer_cast< ModbusMessagePtr >( message );
//          if( modbusMessage )
//          {
//               FMT_LOG_TRACE( "ModbusTcpMaster::ModbusMessage" )
//               if( socket_->is_open() )
//               {
//                    StartMessageTask();
//               }
//               else
//               {
//                    StartConnectTask();
//               }
//               return;
//          }
//          FMT_LOG_TRACE( "ModbusTcpMaster::Receive unsupported message" )
//     }
//
//private:
//     void MessageProcess( const ModbusMessagePtr& message )
//     {
//          std::lock_guard< std::mutex > lock( m_ );
//          messageQueue_.push( message );
//          if( currentMessage_ )
//          {
//               FMT_LOG_DEBUG( "message in queue {}", messageQueue_.size() );
//               return;
//          }
//
//          if( socket_->is_open() )
//          {
//               StartMessageTask();
//          }
//          else
//          {
//               StartConnectTask();
//          }
//     }
//
//     void StartMessageTask()
//     {
//
//     }
//
//     void StartConnectTask()
//     {
//          FMT_LOG_TRACE( "ModbusTcpMaster::StartConnectTask" )
//          Weak weak = GetWeak();
//          socket_->async_connect( ep_, [ weak ]( asio::error_code ec )
//          {
//               Ptr self = weak.lock();
//               if( !self )
//               {
//                    FMT_LOG_CRIT( "ModbusTcpMaster: actor was deleted" )
//                    return;
//               }
//
//               if( ( asio::error::eof == ec ) || ( asio::error::connection_reset == ec ) )
//               {
//                    FMT_LOG_INFO( "ModbusTcpMaster: disconnected: {}", ec.message() )
//                    return;
//               }
//               FMT_LOG_INFO( "ModbusTcpMaster: connect ok" )
//          });
//     }
//
//
//private:
//     TcpSocketUPtr socket_;
//     asio::ip::tcp::endpoint ep_;
//     std::mutex m_;
//     std::queue< ModbusMessagePtr > messageQueue_;
//     ModbusMessagePtr currentMessage_;
//};

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