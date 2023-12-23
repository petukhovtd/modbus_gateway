#include "modbus_tcp_client.h"

#include <common/fmt_logger.h>

namespace test
{

ModbusTcpClient::ModbusTcpClient( const modbus_gateway::ContextPtr& context, const asio::ip::address& addr,
                                  asio::ip::port_type port )
: socket_( std::make_unique< modbus_gateway::TcpSocketUPtr::element_type >( *context ) )
, ep_( asio::ip::address( addr ), port )
{}

asio::error_code ModbusTcpClient::Connect()
{
     asio::error_code ec;
     socket_->connect( ep_, ec );
     return ec;
}

asio::error_code ModbusTcpClient::Disconnect()
{
     asio::error_code ec;
     socket_->cancel( ec );
     return ec;
}

void ModbusTcpClient::Process( const modbus_gateway::ModbusBufferPtr& sendModbusBuffer,
                               const modbus_gateway::ModbusBufferPtr& receiveModbusBuffer )
{
     socket_->async_send( asio::buffer( sendModbusBuffer->begin().base(), sendModbusBuffer->GetAduSize() ),
                          [ = ]( asio::error_code ec, size_t size )
                          {
                               if( ec )
                               {
                                    return;
                               }
                               FMT_LOG_INFO( "ModbusTcpClient::Send {} bytes", size )
                               socket_->async_receive(
                               asio::buffer( receiveModbusBuffer->begin().base(), sendModbusBuffer->GetAduSize() ),
                               [ receiveModbusBuffer ]( asio::error_code ec, size_t size )
                               {
                                    if( ec )
                                    {
                                         return;
                                    }
                                    FMT_LOG_INFO( "ModbusTcpClient::Receive {} bytes", size )
                                    receiveModbusBuffer->SetAduSize( size );
                               } );
                          } );
}

}
