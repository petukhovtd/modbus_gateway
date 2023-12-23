#ifndef MODBUS_GATEWAY_TCP_SERVER_CONFIG_H
#define MODBUS_GATEWAY_TCP_SERVER_CONFIG_H

#include <config/i_transport_config.h>
#include <config/trace_deep.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

class TcpServerConfig: public ITransportConfig
{
public:
     explicit TcpServerConfig( TracePath& tracePath, const nlohmann::json::value_type& obj );

     explicit TcpServerConfig( const asio::ip::address& address, asio::ip::port_type port );

     explicit TcpServerConfig( asio::ip::port_type port );

     ~TcpServerConfig() override = default;

     const asio::ip::address& GetAddress() const;

     asio::ip::port_type GetPort() const;

private:
     asio::ip::address address_;
     asio::ip::port_type port_;
};

}


#endif //MODBUS_GATEWAY_TCP_SERVER_CONFIG_H
