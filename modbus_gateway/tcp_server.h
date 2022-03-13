#ifndef MODBUS_GATEWAY_TCP_SERVER_H
#define MODBUS_GATEWAY_TCP_SERVER_H

#include <exchange/actor_helper.h>
#include <asio.hpp>

#include <types.h>
#include <modbus_tcp_slave.h>

#include <unordered_map>
#include <memory>
#include <mutex>

namespace modbus_gateway
{

class TcpServer
          : public exchange::ActorHelper< TcpServer >
{
     using TcpClientPtr = std::shared_ptr< ModbusTcpSlave >;
     using ClientDb = std::unordered_map< exchange::ActorId, TcpClientPtr >;
public:
     explicit TcpServer( ContextPtr  context, asio::ip::port_type port, TimeoutMs clientTimeout, const RouterPtr& router );

     void Receive( const exchange::MessagePtr& message ) override;

     void Start();

     void Stop();

     ~TcpServer() override;

private:
     void AcceptTask();

     void ClientDisconnect( exchange::ActorId clientId );

private:
     ContextPtr context_;
     asio::ip::tcp::acceptor acceptor_;
     TimeoutMs clientTimeout_;
     RouterPtr router_;
     std::mutex mutex_;
     ClientDb clientDb_;
};

}

#endif
