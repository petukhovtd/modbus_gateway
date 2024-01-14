#pragma once

#include <exchange/actor_helper.h>
#include <exchange/exchange.h>

#include <message/modbus_message.h>

#include <functional>

namespace test {

class ModbusMessageActor : public exchange::ActorHelper<ModbusMessageActor> {
public:
  using Handler = std::function<modbus_gateway::ModbusMessagePtr(const modbus_gateway::ModbusMessagePtr &)>;

  explicit ModbusMessageActor(const exchange::ExchangePtr &exchange);

  ~ModbusMessageActor() override = default;

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;
  void ResetId() override;
  exchange::ActorId GetId() override;

  void SetHandler(const Handler &handler);

private:
  exchange::ActorId id_;
  exchange::ExchangeWeak exchange_;
  Handler handler_;
};

}// namespace test
