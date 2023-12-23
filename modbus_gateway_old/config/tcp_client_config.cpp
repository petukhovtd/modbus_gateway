#include "tcp_client_config.h"
#include "extractor.h"

namespace modbus_gateway
{
TcpClientConfig::TcpClientConfig( TracePath& tracePath, const nlohmann::json::value_type& obj )
: ITransportConfig( TransportType::TCP )
, ModbusClientConfig( tracePath, obj )
, address_( ExtractIpAddress( tracePath, obj ) )
, port_( ExtractIpPort( tracePath, obj ) )
{}

TcpClientConfig::TcpClientConfig( const asio::ip::address& address, asio::ip::port_type port,
                                  const std::vector< UnitIdRange >& unitIdRanges )
: ITransportConfig( TransportType::TCP )
, ModbusClientConfig( unitIdRanges )
, address_( address )
, port_( port )
{

}

const asio::ip::address& TcpClientConfig::GetAddress() const
{
     return address_;
}

asio::ip::port_type TcpClientConfig::GetPort() const
{
     return port_;
}


}
