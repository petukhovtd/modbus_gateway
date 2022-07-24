#include <common/fmt_logger.h>
#include <tcp_server.h>
#include <modbus_tcp_slave.h>

#include <exchange/exchange.h>

#include <chrono>

using namespace modbus_gateway;

class EchoActor: public exchange::ActorHelper< EchoActor >
{
public:
     ~EchoActor() override = default;

     void Receive( const exchange::MessagePtr& message ) override
     {
          FMT_LOG_TRACE( "EchoActor: " )
          auto targetMessage = dynamic_cast< ModbusMessage* >( message.get() );
          if( targetMessage )
          {
               FMT_LOG_TRACE( "EchoActor: modbus" )
               if( targetMessage->GetModbusMessageInfo().unitId == 1 )
               {
                    const auto res = exchange::Exchange::Send( targetMessage->GetModbusMessageInfo().masterId, message );
                    FMT_LOG_TRACE( "EchoActor: echo id {} res {}", targetMessage->GetModbusMessageInfo().masterId, res )
               }
               return;
          }
          FMT_LOG_TRACE( "EchoActor: no" )
     }
};

class Router: public IRouter
{
public:
     Router()
     : echo_( EchoActor::Create() )
     , id_( exchange::Exchange::Insert( echo_ ) )
     {}

     ~Router() override = default;

     exchange::ActorId Route( modbus::UnitId id ) const override
     {
          FMT_LOG_TRACE( "Route: id {}", id )
          return id_;
     }

     EchoActor::Ptr echo_;
     exchange::ActorId id_;
};

class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
{
public:
     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port )
     : socket_( std::make_unique< TcpSocketUPtr::element_type >( *context ) )
     , ep_( addr, port )
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
          auto targetMessage = dynamic_cast< ModbusMessage* >( message.get() );
          if( targetMessage )
          {
               FMT_LOG_TRACE( "ModbusTcpMaster::ModbusMessage" )
               if( socket_->is_open() )
               {
                    StartMessageTask();
               }
               else
               {
                    StartConnectTask();
               }
               return;
          }
          FMT_LOG_TRACE( "ModbusTcpMaster::Receive unsupported message" )
     }

private:
     void StartMessageTask()
     {

     }

     void StartConnectTask()
     {
          FMT_LOG_TRACE( "ModbusTcpMaster::StartConnectTask" )
          if( isConnected_ )
          {
               return;
          }
          isConnected_.store( true );
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
                    return;
               }
               FMT_LOG_INFO( "ModbusTcpMaster: connect ok" )
          });
     }


private:
     TcpSocketUPtr socket_;
     asio::ip::tcp::endpoint ep_;
     std::atomic< bool > isConnected_;

};

int main()
{
     TimeoutMs timeoutMs = 1000;
     RouterPtr router = std::make_shared< Router >();
     FmtLogger::SetLogLevel( FmtLogger::LogLevel::Trace );
     ContextPtr context = std::make_shared< ContextPtr::element_type >();
     TcpServer::Ptr tcpServer = TcpServer::Create( context, 1234, timeoutMs, router );
     exchange::Exchange::Insert( tcpServer );
     tcpServer->Start();
     context->run();
     return 0;
}