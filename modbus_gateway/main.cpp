#include <common/fmt_logger.h>
#include <tcp_server.h>
#include <modbus_tcp_slave.h>

#include <exchange/exchange.h>

using namespace modbus_gateway;

int main()
{
     {
          std::vector< uint8_t > v = { 1, 2, 3 };
          fmt::print( "{}", fmt::join( v, ",") );
          return 0;
     }
     FmtLogger::SetLogLevel( FmtLogger::LogLevel::Trace );
     ContextPtr context = std::make_shared< ContextPtr::element_type >();
     TcpServer::Ptr tcpServer = TcpServer::Create( context, 1234 );
     exchange::Exchange::Insert( tcpServer );
     tcpServer->Start();
     context->run();
     return 0;
}