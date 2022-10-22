#include <common/fmt_logger.h>
#include <common/limit_queue.h>
#include <modbus_tcp_server.h>
#include <modbus_tcp_connection.h>
#include <modbus_tcp_master.h>
#include <config/config.h>
#include <config/invalid_value_exception.h>
#include <exchange/exchange.h>

#include <iostream>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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



void jsontest()
{
     std::ifstream file( "../../modbus_gateway/config_example.json" );
     if( !file )
     {
          std::cerr << "no file" << std::endl;
          return;
     }
     try
     {
          Config config( file );
     }
     catch( const InvalidValueException& e )
     {
          std::cout << e.what() << '\n'
                    << "path: " << e.GetFullPath() << '\n'
          << "value: " << e.GetValue() << '\n';
     }

}

int main()
{
     jsontest();
     return 0;

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