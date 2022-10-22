#include "tcp_server_config.h"
#include "extractor.h"

namespace modbus_gateway
{

TcpServerConfig::TcpServerConfig( TracePath& tracePath, const nlohmann::json::value_type::value_type& obj )
: ITransportConfig( TransportType::TCP )
, address_()
, port_( ExtractIpPort( tracePath, obj ) )
{
     const auto& addr = ExtractIpAddressOptional( tracePath, obj );
     if( addr.has_value() )
     {
          address_ = addr.value();
     }
     else
     {
          address_ = asio::ip::address_v4::any();
     }
}

TcpServerConfig::TcpServerConfig( const asio::ip::address& address, asio::ip::port_type port )
: ITransportConfig( TransportType::TCP )
, address_( address )
, port_( port )
{}

TcpServerConfig::TcpServerConfig( asio::ip::port_type port )
: ITransportConfig( TransportType::TCP )
, address_( asio::ip::address_v4::any() )
, port_( port )
{}

const asio::ip::address& TcpServerConfig::GetAddress() const
{
     return address_;
}

asio::ip::port_type TcpServerConfig::GetPort() const
{
     return port_;
}

}
