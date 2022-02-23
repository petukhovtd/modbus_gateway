#include <modbus_tcp_slave.h>
#include <common/fmt_logger.h>
#include <messages/client_disconnect_message.h>

#include <exchange/exchange.h>

#include <modbus/modbus_buffer.h>

using namespace asio;

namespace modbus_gateway
{

ModbusTcpSlave::ModbusTcpSlave( SocketPtr socket, exchange::ActorId serverId )
          : socket_( std::move( socket ) )
          , serverId_( serverId )
{}

void ModbusTcpSlave::Receive( const exchange::MessagePtr& )
{

}

void ModbusTcpSlave::Start()
{
     assert( GetId().has_value() );
     FMT_LOG_INFO( "Start TcpClient {}:{}, server id {}, client id {}",
                   socket_->remote_endpoint().address().to_string(),
                   socket_->remote_endpoint().port(), serverId_, GetId().value() )
     PushReadTask();
}

void ModbusTcpSlave::Stop()
{
     error_code ec;
     socket_->cancel( ec );
     socket_->close( ec );
}

ModbusTcpSlave::~ModbusTcpSlave()
{
     Stop();
}

void ModbusTcpSlave::PushReadTask()
{
     DataPtr data = std::make_shared< DataPtr::element_type >( DataSize );
     Weak weak = GetWeak();
     socket_->async_receive( buffer( *data ), [ weak, data ]( const error_code& ec, size_t size )
     {
          Ptr self = weak.lock();
          if( !self )
          {
               FMT_LOG_CRIT( "TcpClient actor was deleted" )
               return;
          }
          if( !ec )
          {
               FMT_LOG_TRACE( "TcpClient read {} bytes", size )
               data->resize( size );
               self->PushWriteTask( data );
               self->PushReadTask();
               return;
          }
          if( ( error::eof == ec ) || ( error::connection_reset == ec ) )
          {
               FMT_LOG_INFO( "TcpClient client disconnect: {}", ec.message() )

               exchange::Exchange::Send( self->serverId_, ClientDisconnectMessage::Create( self->GetId().value() ) );
               return;
          }

          FMT_LOG_ERROR( "TcpClient read failed: {}", ec.message() )
     } );
}

void ModbusTcpSlave::PushWriteTask( const DataPtr& data )
{
     socket_->async_send( buffer( *data ), [ data ]( const error_code& ec, size_t size )
     {
          if( ec )
          {
               FMT_LOG_ERROR( "TcpClient write error: {}", ec.message() )
               return;
          }
          FMT_LOG_TRACE( "TcpClient write {}", size )
     } );
}


}
