#ifndef MODBUS_GATEWAY_TCP_CLIENT_CONFIG_H
#define MODBUS_GATEWAY_TCP_CLIENT_CONFIG_H

#include <config/i_transport_config.h>
#include <config/trace_path.h>
#include <config/modbus_client_config.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

class TcpClientConfig: public ITransportConfig, public ModbusClientConfig
{
public:
     explicit TcpClientConfig( TracePath& tracePath, const nlohmann::json::value_type& obj );

     explicit TcpClientConfig( const asio::ip::address& address, asio::ip::port_type port,
                               const std::vector< UnitIdRange >& unitidRanges );

     ~TcpClientConfig() override = default;

     const asio::ip::address& GetAddress() const;

     asio::ip::port_type GetPort() const;

private:
     asio::ip::address address_;
     asio::ip::port_type port_;

};

}

#endif //MODBUS_GATEWAY_TCP_CLIENT_CONFIG_H
