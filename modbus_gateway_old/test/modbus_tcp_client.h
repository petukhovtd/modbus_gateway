#ifndef MODBUS_GATEWAY_MODBUS_TCP_CLIENT_H
#define MODBUS_GATEWAY_MODBUS_TCP_CLIENT_H

#include <types.h>

namespace test
{

class ModbusTcpClient
{
public:
     ModbusTcpClient( const modbus_gateway::ContextPtr& context, const asio::ip::address& addr,
                      asio::ip::port_type port );

     asio::error_code Connect();

     asio::error_code Disconnect();

     void Process( const modbus_gateway::ModbusBufferPtr& sendModbusBuffer,
                   const modbus_gateway::ModbusBufferPtr& receiveModbusBuffer );

private:
     modbus_gateway::TcpSocketUPtr socket_;
     modbus_gateway::TcpEndpoint ep_;
};

}

#endif //MODBUS_GATEWAY_MODBUS_TCP_CLIENT_H
