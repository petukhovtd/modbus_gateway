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
               const auto res =exchange::Exchange::Send( targetMessage->GetModbusMessageInfo().masterId, message );
               FMT_LOG_TRACE( "EchoActor: echo id {} res {}", targetMessage->GetModbusMessageInfo().masterId, res )
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