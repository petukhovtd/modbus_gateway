#ifndef MODBUS_GATEWAY_MODBUS_TCP_SERVER_H
#define MODBUS_GATEWAY_MODBUS_TCP_SERVER_H

#include <exchange/actor_helper.h>
#include <asio.hpp>

#include <types.h>
#include <modbus_tcp_connection.h>
#include <i_modbus_slave.h>

#include <unordered_map>
#include <memory>
#include <mutex>

namespace modbus_gateway
{

class ModbusTcpServer final
          : public exchange::ActorHelper< ModbusTcpServer >,
          public IModbusSlave
{
     using TcpClientPtr = std::shared_ptr< ModbusTcpConnection >;
     using ClientDb = std::unordered_map< exchange::ActorId, TcpClientPtr >;
public:
     ModbusTcpServer( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, RouterPtr router );

     void Receive( const exchange::MessagePtr& message ) override;

     void Start() override;

     void Stop();

     ~ModbusTcpServer() override;

private:
     void AcceptTask();

     void ClientDisconnect( exchange::ActorId clientId );

private:
     TcpAcceptor acceptor_;
     RouterPtr router_;
     std::mutex mutex_;
     ClientDb clientDb_;
};

}

#endif
