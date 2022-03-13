#include <tcp_server.h>
#include <common/fmt_logger.h>
#include <messages/client_disconnect_message.h>

#include <exchange/exchange.h>

using namespace asio;

namespace modbus_gateway
{

TcpServer::TcpServer( ContextPtr context, ip::port_type port, TimeoutMs clientTimeout, const RouterPtr& router )
          : context_( std::move( context ) )
          , acceptor_( *context_, ip::tcp::endpoint( ip::address_v4::any(), port ) )
          , clientTimeout_( clientTimeout )
          , router_( router )
{
     assert( context_ );
     assert( router_ );
}

void TcpServer::Receive( const exchange::MessagePtr& message )
{
     auto ptr = dynamic_cast< ClientDisconnectMessage* >( message.get() );
     if( ptr )
     {
          ClientDisconnect( ptr->GetClientId() );
          return;
     }
}

void TcpServer::Start()
{
     assert( GetId().value() );
     AcceptTask();
}

void TcpServer::Stop()
{
     error_code ec;
     acceptor_.cancel( ec );
     acceptor_.close( ec );
     {
          std::unique_lock< std::mutex > lock( mutex_ );
          for( const auto&[id, client]: clientDb_ )
          {
               client->Stop();
          }
     }
}

TcpServer::~TcpServer()
{
     Stop();
}

void TcpServer::AcceptTask()
{
     SocketPtr socket = std::make_shared< SocketPtr::element_type >( *context_ );
     Weak weak = GetWeak();
     acceptor_.async_accept( *socket, [ weak, socket ]( const error_code& ec )
     {
          if( ec )
          {
               FMT_LOG_ERROR( "TcpServer accept failed: {}", ec.message() )
               return;
          }
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "TcpServer actor was deleted" )
               return;
          }
          FMT_LOG_INFO( "TcpServer connect from {}:{}", socket->remote_endpoint().address().to_string(),
                        socket->remote_endpoint().port() )
          auto tcpClient = ModbusTcpSlave::Create( self->GetId().value(), socket, self->clientTimeout_, self->router_ );
          auto clientId = exchange::Exchange::Insert( tcpClient );
          {
               std::unique_lock< std::mutex > lock( self->mutex_ );
               self->clientDb_[ clientId ] = tcpClient;
          }
          tcpClient->Start();
          self->AcceptTask();
     } );
}

void TcpServer::ClientDisconnect( exchange::ActorId clientId )
{
     std::unique_lock< std::mutex > lock( mutex_ );
     FMT_LOG_INFO( "TcpServer: remove {}", clientId )
     clientDb_.erase( clientId );
}

}
