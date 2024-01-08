#pragma once

#include <transport/modbus_tcp_connection.h>
#include <transport/i_modbus_slave.h>

#include <common/types_asio.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace modbus_gateway {

class ModbusTcpServer final : public exchange::ActorHelper<ModbusTcpServer>, public IModbusSlave {
  using TcpClientPtr = std::shared_ptr<ModbusTcpConnection>;
  using ClientDb = std::unordered_map<exchange::ActorId, TcpClientPtr>;

public:
  ModbusTcpServer(const exchange::ExchangePtr &exchange, const ContextPtr &context, const asio::ip::address &addr,
                  asio::ip::port_type port, const RouterPtr &router);

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  exchange::ActorId GetId() override;

  void Start() override;

  void Stop() override;

  ~ModbusTcpServer() override;

private:
  void AcceptTask();

  void ClientDisconnect(exchange::ActorId clientId);

private:
  std::atomic<exchange::ActorId> id_;
  exchange::ExchangePtr exchange_;
  TcpAcceptor acceptor_;
  RouterPtr router_;
  std::mutex mutex_;
  ClientDb clientDb_;
};

}// namespace modbus_gateway
