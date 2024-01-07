#pragma once

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <message/modbus_message.h>

namespace test {

class ModbusMessageSender : public exchange::ActorHelper<ModbusMessageSender> {
public:
  explicit ModbusMessageSender(const exchange::ExchangePtr &exchange);

  ~ModbusMessageSender() override = default;

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  void SendTo(const modbus_gateway::ModbusBufferPtr &modbusBuffer, exchange::ActorId target);

  modbus_gateway::ModbusMessagePtr receiveMessage = nullptr;

private:
  exchange::ActorId id_;
  exchange::ExchangePtr exchange_;
  modbus::TransactionId transactionIdGenerator_;
};

}// namespace test
