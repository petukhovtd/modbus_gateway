#include "modbus_gateway.h"
#include "config/tcp_client_config.h"
#include "config/tcp_server_config.h"
#include "router.h"

#include "exchange/i_actor.h"
#include "exchange/exchange.h"
#include "modbus_tcp_master.h"
#include "modbus_tcp_server.h"
#include "i_modbus_slave.h"

#include <iostream>

namespace modbus_gateway
{

struct Client
{
     TransportConfigPtr config;
     exchange::ActorPtr actor;
};

struct Server
{
     TransportConfigPtr config;
     ModbusSlavePtr server;
};

std::vector< Client > MakeClients( const std::vector< TransportConfigPtr >& clientsConfig, const ContextPtr& context )
{
     using namespace std::chrono_literals;

     std::vector< Client > clients;
     clients.reserve( clientsConfig.size() );

     for( const auto& clientConfigPtr: clientsConfig )
     {
          switch( clientConfigPtr->GetType() )
          {
               case TCP:
               {
                    const auto& clientConfig = std::dynamic_pointer_cast< TcpClientConfig >( clientConfigPtr );
                    if( !clientConfig )
                    {
                         throw std::logic_error( "BUG! cast to TcpClientConfig failed" );
                    }
                    ModbusTcpMaster::Ptr tcpMaster = ModbusTcpMaster::Create( context, clientConfig->GetAddress(),
                                                                              clientConfig->GetPort(), 1000ms );
                    const exchange::ActorId actorId = exchange::Exchange::Insert( tcpMaster );
                    FMT_LOG_DEBUG( "create modbus tcp master address {}, port {}, timeout {}, actor id {}",
                                   clientConfig->GetAddress().to_string(), clientConfig->GetPort(), 1000, actorId )
                    Client client = { clientConfig, tcpMaster };
                    if( clientConfig->GetUnitIds().empty() )
                    {
                         clients.insert( clients.begin(), client );
                    }
                    else
                    {
                         clients.push_back( client );
                    }
               }
                    break;
               default:
                    throw std::logic_error( "BUG! unknown client type" );
          }
     }

     return clients;
}

std::shared_ptr< Router > MakeRouter( const std::vector< Client >& clients )
{
     std::shared_ptr< Router > router = nullptr;

     if( clients.empty() )
     {
          return router;
     }

     auto clientIt = clients.begin();

     {
          const auto& clientConfig = std::dynamic_pointer_cast< ModbusClientConfig >( clientIt->config );
          if( !clientConfig )
          {
               throw std::logic_error( "BUG! cast to ModbusClientConfig failed" );
          }

          // default id
          if( clientConfig->GetUnitIds().empty() )
          {
               if( clientIt->actor->GetId().has_value() )
               {
                    router = std::make_shared< Router >( clientIt->actor->GetId().value() );
                    FMT_LOG_DEBUG( "route by default to actor id {}", clientIt->actor->GetId().value() )
               }
               else
               {
                    throw std::logic_error( "BUG! default client actor don't have id" );
               }
               ++clientIt;
          }
          else
          {
               router = std::make_shared< Router >();
          }
     }

     for( ; clientIt != clients.end(); ++ clientIt )
     {
          const auto& clientConfig = std::dynamic_pointer_cast< ModbusClientConfig >( clientIt->config );
          if( !clientConfig )
          {
               throw std::logic_error( "BUG! cast to ModbusClientConfig failed" );
          }

          if( clientConfig->GetUnitIds().empty() )
          {
               throw std::logic_error( "BUG! more one default client" );
          }

          if( !clientIt->actor->GetId().has_value() )
          {
               throw std::logic_error( "BUG! client actor don't have id" );
          }

          for( const auto& unitIdRange: clientConfig->GetUnitIds() )
          {
               for( modbus::UnitId unitId = unitIdRange.begin; unitId <= unitIdRange.end; ++unitId )
               {
                    router->Set( unitId, clientIt->actor->GetId().value() );
                    FMT_LOG_DEBUG( "route unit id {} to actor id {}", unitId, clientIt->actor->GetId().value() )
               }
          }
     }

     return router;
}

std::vector< Server > MakeServers( const std::vector< TransportConfigPtr >& serversConfig, const ContextPtr& context, const RouterPtr& router )
{
     std::vector< Server > servers;
     servers.reserve( serversConfig.size() );

     for( const auto& serverConfigPtr: serversConfig )
     {
          switch( serverConfigPtr->GetType() )
          {
               case TCP:
               {
                    const auto& serverConfig = std::dynamic_pointer_cast< TcpServerConfig >( serverConfigPtr );
                    if( !serverConfig )
                    {
                         throw std::logic_error( "BUG! cast to TcpServerConfig failed" );
                    }
                    ModbusTcpServer::Ptr tcpServer = ModbusTcpServer::Create( context, serverConfig->GetAddress(),
                                                                              serverConfig->GetPort(), router );
                    const exchange::ActorId tcpServerId = exchange::Exchange::Insert( tcpServer );
                    FMT_LOG_DEBUG( "create modbus tcp server address {}, port {}, actor id {}",
                                   serverConfig->GetAddress().to_string(), serverConfig->GetPort(), tcpServerId )
                    Server server = { serverConfig, tcpServer };
                    servers.push_back( server );
               }
                    break;
               default:
                    throw std::logic_error( "BUG! unknown client type" );
          }
     }

     return servers;
}

int ModbusGateway( const Config& config )
{
     ContextPtr context = std::make_shared< ContextPtr::element_type >();
     auto work = asio::executor_work_guard( context->get_executor() );

     asio::signal_set signalSet( *context );
     signalSet.add( SIGINT );
     signalSet.add( SIGILL );
     signalSet.add( SIGABRT );
     signalSet.add( SIGFPE );
     signalSet.add( SIGSEGV );
     signalSet.add( SIGTERM );


     signalSet.async_wait( [ context ]( const asio::error_code& ec, int signalNumber )
                           {
                                if( !ec )
                                {
                                     FMT_LOG_INFO( "signal: {}", signalNumber )
                                     context->stop();
                                }
                           } );

     std::vector< Client > clients = MakeClients( config.GetClients(), context );
     if( clients.empty() )
     {
          throw std::logic_error( "BUG! clients empty" );
     }

     std::shared_ptr< Router > router = MakeRouter( clients );
     if( !router )
     {
          throw std::logic_error( "BUG! clients empty" );
     }

     std::vector< Server > servers = MakeServers( config.GetServers(), context, router );
     if( servers.empty() )
     {
          throw std::logic_error( "BUG! servers empty" );
     }

     FMT_LOG_INFO( "start servers" );
     for( auto& server: servers )
     {
          server.server->Start();
     }

     FMT_LOG_INFO( "starting" )
     context->run();
     FMT_LOG_INFO( "stopping" )

     return EXIT_SUCCESS;
}

}