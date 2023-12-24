#pragma once

#include <transport/modbus_tcp_connection.h>

#include <common/types_asio.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <unordered_map>
#include <memory>
#include <mutex>

namespace modbus_gateway {

class ModbusTcpServer final : public exchange::ActorHelper<ModbusTcpServer> {
    using TcpClientPtr = std::shared_ptr<ModbusTcpConnection>;
    using ClientDb = std::unordered_map<exchange::ActorId, TcpClientPtr>;
public:
    ModbusTcpServer(const exchange::ExchangePtr &exchange, const ContextPtr &context, const asio::ip::address &addr,
                    asio::ip::port_type port, const RouterPtr &router);

    void Receive(const exchange::MessagePtr &message) override;

    void Start();

    void Stop();

    ~ModbusTcpServer() override;

private:
    void AcceptTask();

    void ClientDisconnect(exchange::ActorId clientId);

    const std::string &GetIdStr() const;

private:
    exchange::ExchangePtr exchange_;
    TcpAcceptor acceptor_;
    RouterPtr router_;
    std::mutex mutex_;
    ClientDb clientDb_;
};

}
