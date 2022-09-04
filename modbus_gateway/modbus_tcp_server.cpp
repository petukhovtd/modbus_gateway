#include "modbus_tcp_server.h"
#include <common/fmt_logger.h>
#include <messages/client_disconnect_message.h>

#include <exchange/exchange.h>

using namespace asio;

namespace modbus_gateway
{

ModbusTcpServer::ModbusTcpServer( const ContextPtr& context, const asio::ip::address& addr, ip::port_type port,
                                  RouterPtr router )
: acceptor_( *context, TcpEndpoint( addr, port ) )
, router_( std::move( router ) )
{
     assert( router_ );
}

void ModbusTcpServer::Receive( const exchange::MessagePtr& message )
{
     auto ptr = dynamic_cast< ClientDisconnectMessage* >( message.get() );
     if( ptr )
     {
          ClientDisconnect( ptr->GetClientId() );
          return;
     }
}

void ModbusTcpServer::Start()
{
     assert( GetId().value() );
     AcceptTask();
}

void ModbusTcpServer::Stop()
{
     error_code ec;
     acceptor_.close( ec );
     {
          std::unique_lock< std::mutex > lock( mutex_ );
          for( const auto&[id, client]: clientDb_ )
          {
               client->Stop();
          }
     }
}

ModbusTcpServer::~ModbusTcpServer()
{
     Stop();
}

void ModbusTcpServer::AcceptTask()
{
     TcpSocketPtr socket = std::make_shared< TcpSocketPtr::element_type >( acceptor_.get_executor() );
     Weak weak = GetWeak();
     acceptor_.async_accept( *socket, [ weak, socket ]( error_code ec )
     {
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "ModbusTcpServer: accept: actor was deleted" )
               return;
          }

          if( ec )
          {
               FMT_LOG_ERROR( "ModbusTcpServer: accept: error: {}", ec.message() )
               if( error::operation_aborted == ec )
               {
                    FMT_LOG_INFO( "ModbusTcpServer: accept: canceled" )
                    return;
               }
               FMT_LOG_TRACE( "ModbusTcpServer: accept: start accept task" )
               self->AcceptTask();
               return;
          }
          FMT_LOG_INFO( "ModbusTcpServer: accept: connect from {}:{}", socket->remote_endpoint().address().to_string(),
                        socket->remote_endpoint().port() )
          auto tcpClient = ModbusTcpConnection::Create( self->GetId().value(), socket, self->router_ );
          auto clientId = exchange::Exchange::Insert( tcpClient );
          {
               std::unique_lock< std::mutex > lock( self->mutex_ );
               self->clientDb_[ clientId ] = tcpClient;
          }
          tcpClient->Start();
          self->AcceptTask();
     } );
}

void ModbusTcpServer::ClientDisconnect( exchange::ActorId clientId )
{
     std::unique_lock< std::mutex > lock( mutex_ );
     FMT_LOG_INFO( "ModbusTcpServer: remove client {}", clientId )
     clientDb_.erase( clientId );
}

}
